#ifndef __PUBLIC_TYPEDEF_H__
#define __PUBLIC_TYPEDEF_H__

/** @defgroup HSF-Usual-DataType*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include <math.h>  
#include "stdarg.h" // vsnprintf函数头文件



#define FALSE         	0
#define TRUE          	1


#define RET_SUCCESS		0
#define RET_FAILURE		(-1)


/* \brief Function pointer define */
typedef void (*pFun) (void);



#define BIT0              (0x01ul<<0)   
#define BIT1              (0x01ul<<1) 
#define BIT2              (0x01ul<<2)
#define BIT3              (0x01ul<<3)
#define BIT4              (0x01ul<<4)
#define BIT5              (0x01ul<<5)
#define BIT6              (0x01ul<<6)
#define BIT7              (0x01ul<<7)
#define BIT8              (0x01ul<<8)
#define BIT9              (0x01ul<<9)
#define BIT10             (0x01ul<<10)
#define BIT11             (0x01ul<<11)
#define BIT12             (0x01ul<<12)
#define BIT13             (0x01ul<<13)
#define BIT14             (0x01ul<<14)
#define BIT15             (0x01ul<<15)
#define BIT16            (0x01ul<<16)
#define BIT17            (0x01ul<<17)
#define BIT18            (0x01ul<<18)
#define BIT19            (0x01ul<<19)
#define BIT20            (0x01ul<<20)
#define BIT21            (0x01ul<<21)
#define BIT22            (0x01ul<<22)
#define BIT23            (0x01ul<<23)
#define BIT24           (0x01ul<<24)
#define BIT25           (0x01ul<<25)
#define BIT26           (0x01ul<<26)
#define BIT27           (0x01ul<<27)
#define BIT28           (0x01ul<<28)
#define BIT29           (0x01ul<<29)
#define BIT30           (0x01ul<<30)
#define BIT31           (0x01ul<<31)



#define HBYTE(x) 				((uint8_t)(((uint16_t)(x) >> 8) & 0xff))
#define LBYTE(x) 				((uint8_t)(x))
#define COMB_2BYTE(high, low) 	((((uint16_t)(high))<<8) | (((uint16_t)(low)) & 0x00ff))//2个u8合成一个u16
#define SWAP_WORD(x) 			((((uint16_t)(x))>>8) | (((uint16_t)(x))<<8)) //高低8位转换


#define U32_BYTE0(data)      ((uint8_t *)(&(data)))[0]
#define U32_BYTE1(data)      ((uint8_t *)(&(data)))[1]
#define U32_BYTE2(data)      ((uint8_t *)(&(data)))[2]
#define U32_BYTE3(data)      ((uint8_t *)(&(data)))[3]
#define COMB_4BYTE(byte3, byte2,byte1,byte0) 	(((uint32_t)byte3<<24)|((uint32_t)byte2<<16)|((uint32_t)byte1<<8)|(uint32_t)byte0)//4个u8合成一个u32


/* \brief Bits string defines */
typedef union
{
    struct { char b0:1; char b1:1; char b2:1; char b3:1;
             char b4:1; char b5:1; char b6:1; char b7:1; };
    uint8_t h;
} bits8_t;
typedef union
{
    struct { char b0 :1; char b1 :1; char b2 :1; char b3 :1;
             char b4 :1; char b5 :1; char b6 :1; char b7 :1;
             char b8 :1; char b9 :1; char b10:1; char b11:1;
             char b12:1; char b13:1; char b14:1; char b15:1; };
    uint16_t hh;
} bits16_t;
typedef union
{
    struct { char b0 :1; char b1 :1; char b2 :1; char b3 :1;
             char b4 :1; char b5 :1; char b6 :1; char b7 :1;
             char b8 :1; char b9 :1; char b10:1; char b11:1;
             char b12:1; char b13:1; char b14:1; char b15:1;
             char b16:1; char b17:1; char b18:1; char b19:1;
             char b20:1; char b21:1; char b22:1; char b23:1;
             char b24:1; char b25:1; char b26:1; char b27:1;
             char b28:1; char b29:1; char b30:1; char b31:1; };
    uint32_t hhhh;
} bits32_t;

#ifndef st
	#define st(x)         do { x } while (__LINE__ == -1)
#endif

#ifndef bitmask
  #define bitmask(x)     (1u << (x))
#endif


/**
  * \brief  Translate expression statement or variable to bool value.
  * \param  [in]  x-the statement.
  * \return The result TRUE(1) or FALSE(0) in \a x.
  */

#ifndef boolof
	#define boolof(x)    ((x) ? TRUE : FALSE)
#endif


/**
  * \brief  Returns the number of elements in an array.
  * \param  [in]  x-the defined array name.
  * \return The count of elements in \a x.
  */
#ifndef eleof
	#define eleof(x)     (sizeof(x) / sizeof((x)[0]))
#endif


/**
  * \brief  Returns the offset of member in a structure.
  * \param  [in]  T-the defined structure's name.
  * \param  [in]  member-the member name in defined structure.
  * \return The offset of the specified member in defined structure.
  */
#ifndef offsetof
  #define offsetof(T, member)   (size_t((&((T*)0)->member)))
#endif

#define SIZEOF(s,m) 			(sizeof(((s *)0)->m))













#endif /* __TYPEDEF_H */

