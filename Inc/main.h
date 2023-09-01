/**
  ******************************************************************************
  * @file    main.h
  * @author  Application Team
  * @Version V1.0.0
  * @Date    1-April-2019
  * @brief   Header for main.c file.
  *          This file contains the common defines of the application.
  ******************************************************************************
  */

  /* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

  /* Includes ------------------------------------------------------------------*/
#include "cs32l010_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "log.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
/* USER CODE END Includes */

// L76K 电源使能模式
#define ENABLE_POWER 0
#define DISABLE_POWER 1

/**
 * @enum eCalibRTCStatus_t
 * @brief RTC 模块校准状态
 */
typedef enum {
    eCalibNone = 0x00,
    eCalibComplete = 0x01,
    eUnderCalib = 0x02,
}eCalibRTCStatus_t;

/* Exported functions prototypes ---------------------------------------------*/

void userSetHandle(uint8_t r_address, uint8_t length);

void SystemClock_Config(void);
void Error_Handler(void);

/* Private defines -----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

