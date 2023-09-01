#ifndef _DFROBOTSOFTIIC_H
#define _DFROBOTSOFTIIC_H

#include "main.h"

#define SOFT_SDA_PORT   GPIOD
#define SOFT_SDA_PIN    GPIO_PIN_2
#define SOFT_SCL_PORT   GPIOD
#define SOFT_SCL_PIN    GPIO_PIN_3

void Soft_IIC_Init(uint8_t addr);

void setAddr(uint8_t addr);

void writeReg(uint8_t reg, uint8_t* buf, uint8_t len);
void readReg(uint8_t reg, uint8_t * buf, uint8_t len);

void scan_i2c_address(void);

#endif
