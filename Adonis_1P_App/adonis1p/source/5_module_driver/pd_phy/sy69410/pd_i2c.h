#ifndef PD_I2C
#define PD_I2C

#include <stdint.h>

uint8_t I2CWriteBytes(uint8_t SlaveAddress,uint8_t MemoryAdress,uint8_t wrNumber,uint8_t* wrPointer);
uint8_t I2CReadBytes(uint8_t SlaveAddress,uint8_t MemoryAdress,uint8_t rdNumber,uint8_t* rdPointer);

extern void Set1Reg(uint8_t memory_addr, uint8_t value);
extern uint8_t Get1Reg(uint8_t memory_addr) ;
extern void Set2Reg(uint8_t memory_addr, uint16_t value);
extern uint16_t Get2Reg(uint8_t memory_addr);

#endif
