
#include <string.h>
#include "sha256.h"

static void compress(unsigned int *iv, const uint8_t *data);

int sha256_init(struct sha256 *self)
{
	if (self == 0) {
		return -1;
	}
  
  memset(self, 0, sizeof(*self));
	self->iv[0] = 0x6a09e667;
	self->iv[1] = 0xbb67ae85;
	self->iv[2] = 0x3c6ef372;
	self->iv[3] = 0xa54ff53a;
	self->iv[4] = 0x510e527f;
	self->iv[5] = 0x9b05688c;
	self->iv[6] = 0x1f83d9ab;
	self->iv[7] = 0x5be0cd19;

	return 0;
}

int sha256_update(struct sha256 *self, const uint8_t *data, size_t datalen)
{
	if (self == 0 || data == (void *) 0) {
		return -1;
	} else if (datalen == 0) {
		return 0;
	}

	while (datalen-- > 0) {
		self->leftover[self->leftover_offset++] = *(data++);
		if (self->leftover_offset >= SHA256_BLOCK_SIZE) {
			compress(self->iv, self->leftover);
			self->leftover_offset = 0;
			self->bits_hashed += (SHA256_BLOCK_SIZE << 3);
		}
	}
	return 0;
}

int sha256_final(struct sha256* self, uint8_t digest[32])
{
	unsigned int i;

	if (digest == (uint8_t *) 0 || self == 0) {
		return -1;
	}

	self->bits_hashed += (self->leftover_offset << 3);

	self->leftover[self->leftover_offset++] = 0x80;
	if (self->leftover_offset > (sizeof(self->leftover) - 8)) {
		memset(self->leftover + self->leftover_offset, 0x00, sizeof(self->leftover) - self->leftover_offset);
		compress(self->iv, self->leftover);
		self->leftover_offset = 0;
	}

	memset(self->leftover + self->leftover_offset, 0x00, sizeof(self->leftover) - 8 - self->leftover_offset);
	self->leftover[sizeof(self->leftover) - 1] = (uint8_t)(self->bits_hashed);
	self->leftover[sizeof(self->leftover) - 2] = (uint8_t)(self->bits_hashed >> 8);
	self->leftover[sizeof(self->leftover) - 3] = (uint8_t)(self->bits_hashed >> 16);
	self->leftover[sizeof(self->leftover) - 4] = (uint8_t)(self->bits_hashed >> 24);
	self->leftover[sizeof(self->leftover) - 5] = (uint8_t)(self->bits_hashed >> 32);
	self->leftover[sizeof(self->leftover) - 6] = (uint8_t)(self->bits_hashed >> 40);
	self->leftover[sizeof(self->leftover) - 7] = (uint8_t)(self->bits_hashed >> 48);
	self->leftover[sizeof(self->leftover) - 8] = (uint8_t)(self->bits_hashed >> 56);

	compress(self->iv, self->leftover);
  
	for (i = 0; i < SHA256_STATE_BLOCKS; ++i) {
		unsigned int t = *((unsigned int *) &self->iv[i]);
		*digest++ = (uint8_t)(t >> 24);
		*digest++ = (uint8_t)(t >> 16);
		*digest++ = (uint8_t)(t >> 8);
		*digest++ = (uint8_t)(t);
	}
  
	memset(self, 0, sizeof(*self));

	return 0;
}

static const unsigned int k256[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
	0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
	0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
	0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
	0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
	0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static inline unsigned int ROTR(unsigned int a, unsigned int n)
{
	return (((a) >> n) | ((a) << (32 - n)));
}

#define Sigma0(a)(ROTR((a), 2) ^ ROTR((a), 13) ^ ROTR((a), 22))
#define Sigma1(a)(ROTR((a), 6) ^ ROTR((a), 11) ^ ROTR((a), 25))
#define sigma0(a)(ROTR((a), 7) ^ ROTR((a), 18) ^ ((a) >> 3))
#define sigma1(a)(ROTR((a), 17) ^ ROTR((a), 19) ^ ((a) >> 10))

#define Ch(a, b, c)(((a) & (b)) ^ ((~(a)) & (c)))
#define Maj(a, b, c)(((a) & (b)) ^ ((a) & (c)) ^ ((b) & (c)))

static inline unsigned int BigEndian(const uint8_t **c)
{
	unsigned int n = 0;

	n = (((unsigned int)(*((*c)++))) << 24);
	n |= ((unsigned int)(*((*c)++)) << 16);
	n |= ((unsigned int)(*((*c)++)) << 8);
	n |= ((unsigned int)(*((*c)++)));
	return n;
}

static void compress(unsigned int *iv, const uint8_t *data)
{
	unsigned int a, b, c, d, e, f, g, h;
	unsigned int s0, s1;
	unsigned int t1, t2;
	unsigned int work_space[16];
	unsigned int n;
	unsigned int i;

	a = iv[0]; b = iv[1]; c = iv[2]; d = iv[3];
	e = iv[4]; f = iv[5]; g = iv[6]; h = iv[7];

	for (i = 0; i < 16; ++i) {
		n = BigEndian(&data);
		t1 = work_space[i] = n;
		t1 += h + Sigma1(e) + Ch(e, f, g) + k256[i];
		t2 = Sigma0(a) + Maj(a, b, c);
		h = g; g = f; f = e; e = d + t1;
		d = c; c = b; b = a; a = t1 + t2;
	}

	for ( ; i < 64; ++i) {
		s0 = work_space[(i+1)&0x0f];
		s0 = sigma0(s0);
		s1 = work_space[(i+14)&0x0f];
		s1 = sigma1(s1);

		t1 = work_space[i&0xf] += s0 + s1 + work_space[(i+9)&0xf];
		t1 += h + Sigma1(e) + Ch(e, f, g) + k256[i];
		t2 = Sigma0(a) + Maj(a, b, c);
		h = g; g = f; f = e; e = d + t1;
		d = c; c = b; b = a; a = t1 + t2;
	}

	iv[0] += a; iv[1] += b; iv[2] += c; iv[3] += d;
	iv[4] += e; iv[5] += f; iv[6] += g; iv[7] += h;
}
