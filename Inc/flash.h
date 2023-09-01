/* USER CODE BEGIN Header */
/*!
 * @file  flash.h
 * @brief   This file contains all the function prototypes for
 * @n       the flash.c file
 * @copyright  Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license  The MIT License (MIT)
 * @author  [qsjhyy](yihuan.huang@dfrobot.com)
 * @version  V1.0
 * @date  2022-09-15
 */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_H__
#define __FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* Exported constants macros ---------------------------------------------------------*/

#define FLASH_PROGRAM_ADDRESS_START   0xFE00U   // 64K
#define FLASH_PROGRAM_ADDRESS_END     0x10000U

/* USER CODE END Private defines */

void MY_FLASH_erase(void);

void MY_FLASH_write(uint8_t *pBuffer, uint32_t NumToWrite );

void MY_FLASH_read(uint8_t *const buffer, uint32_t length);

void MY_FLASH_Error_Handler(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_H__ */

