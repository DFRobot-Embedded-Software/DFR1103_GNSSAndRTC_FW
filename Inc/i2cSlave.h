/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2C_SLAVE_H
#define __I2C_SLAVE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "main.h"


/* Private includes ----------------------------------------------------------*/
#define CS32_I2C_ADDR    0x66   // 本机i2c地址

#define I2C_SPEED_RATE   100   // uint khz
#define I2C_IRQ_LEVEL    0x00U

extern volatile uint8_t userSetFlag, userSetCount, userSetBeginReg;

/* Exported functions prototypes ---------------------------------------------*/
void I2C_Init(void);

void HAL_I2C_SlaveCallback(I2C_HandleTypeDef *hi2c);

void slave_i2c_test(void);

/* Private defines -----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __I2C_SLAVE_H */
