
#ifndef SHA256_H
#define SHA256_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHA256_BLOCK_SIZE   (64)
#define SHA256_DIGEST_SIZE  (32)
#define SHA256_STATE_BLOCKS (SHA256_DIGEST_SIZE/4)

struct sha256 {
	unsigned int iv[SHA256_STATE_BLOCKS];
	uint64_t bits_hashed;
	uint8_t leftover[SHA256_BLOCK_SIZE];
	size_t leftover_offset;
};

int sha256_init(struct sha256 *self);

int sha256_update(struct sha256 *self, const uint8_t *data, size_t datalen);

int sha256_final(struct sha256 *self, uint8_t digest[32]);

#ifdef __cplusplus
}
#endif

#endif
