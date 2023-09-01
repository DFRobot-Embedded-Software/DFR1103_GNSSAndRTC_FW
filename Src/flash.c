/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    flash.c
  * @brief   This file provides code for the configuration
  *          of the flash instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "flash.h"

/* USER CODE BEGIN 0 */

static uint32_t uiErrorPage;
static uint32_t uiAddress = 0;

/* USER CODE END 0 */


/* USER CODE BEGIN 1 */

/**
 * @brief flash擦除, 0xFE00U 到 0x10000U
 * 
 */
void MY_FLASH_erase(void)
{
  FLASH_EraseInitTypeDef 	sFlashEraseInit;
	/* Flash page erase from FLASH_PROGRAM_ADDRESS_START to FLASH_PROGRAM_ADDRESS_END*/
	sFlashEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;	// 页擦除 
	sFlashEraseInit.PageAddress = FLASH_PROGRAM_ADDRESS_START; // 开始擦除地址 必须以512Byte为单位
	sFlashEraseInit.NbPages	= (FLASH_PROGRAM_ADDRESS_END - FLASH_PROGRAM_ADDRESS_START)/FLASH_PAGE_SIZE + 1;
	if(HAL_FLASH_Erase(&sFlashEraseInit, &uiErrorPage) != HAL_OK)
	{
    MY_FLASH_Error_Handler();
	}
  uiAddress = FLASH_PROGRAM_ADDRESS_START;
  while (uiAddress < FLASH_PROGRAM_ADDRESS_END)
  {
    if (*(uint8_t*)(uiAddress) != 0xFF) // 判断地址数据是否擦除成功
    {
      /* Error occurred while writing data in Flash memory.
         User can add here some code to deal with this error */
			MY_FLASH_Error_Handler();
			uiAddress = uiAddress + 1;
    }
		else
		{
			uiAddress = uiAddress + 1;
		}
  }
}

/**
 * @brief flash写入
 * 
 * @param pBuffer - 数据指针
 * @param NumToWrite - 字节数数
 */
void MY_FLASH_write(uint8_t *pBuffer, uint32_t NumToWrite )
{
  MY_FLASH_erase();
  uiAddress = FLASH_PROGRAM_ADDRESS_START;	 // 起始写入地址
  uint32_t i;
  for( i=0; i<NumToWrite; i++ )
  {
    uiAddress = FLASH_PROGRAM_ADDRESS_START + i;
    if ((uiAddress) < FLASH_PROGRAM_ADDRESS_END)
      if( HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, uiAddress,*(uint8_t *)(pBuffer+i) ) )
        MY_FLASH_Error_Handler();
  }

  /* 恢复系统滴答定时器 */
  HAL_ResumeTick();
  HAL_Delay(50);
  /* 暂停systemtick以避免系统被systemtick唤醒 */  
  HAL_SuspendTick();
}

/**
 * @brief 从FLASH中读指定长度数据, 都从 0xFE00U 开始读取
 * 
 * @param buffer 存数据的buff
 * @param length 读取数据的长度
 */
void MY_FLASH_read(uint8_t *const buffer, uint32_t length)
{
  uiAddress = FLASH_PROGRAM_ADDRESS_START;	 // 起始写入地址
  uint32_t i;
  uint8_t data;
  for ( i = 0; i < length; i++)
  {
    uiAddress = FLASH_PROGRAM_ADDRESS_START + i;
    if ((uiAddress) < FLASH_PROGRAM_ADDRESS_END) {
      data = *(uint8_t *)(uiAddress);
      buffer[i] = (data & 0xFF);
    }
  }
}

/**
 * @brief flash操作失败打印
 * 
 */
void MY_FLASH_Error_Handler(void)
{
  // printf_log("Error Address is 0x%x\n", uiAddress);
  // printf_log("Error Data is 0x%x\n", *(uint8_t*)(uiAddress));	
}

/* USER CODE END 1 */
