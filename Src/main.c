/*!
 * @file  main.c
 * @brief  这是一个GNSS&RTC模块（DFR1103）专用的固件
 * @details  cs32 用硬件 i2c 、 uart0 和主控通信;
 * @n  cs32 用软件 i2c 与 SD3031 通信, 用 uart1 和 L76K 通信。
 * @n  同时 cs32 用 PA3 控制 L76K_PWR; 用 PB5 控制 L76K_RST;
 * @n  用 PB4 控制 L76K_WAKEUP; 用 PC5 控制 L76K_SET。
 * @n  另外 SD3031 输出 INT 以及 F32K 引脚; L76K 输出 PPS 引脚。
 * @copyright  Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license  The MIT License (MIT)
 * @author  [qsjhyy](yihuan.huang@dfrobot.com)
 * @version  V1.0
 * @date  2023-08-18
 */
 /* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "uart.h"
#include "i2cSlave.h"
#include "DFRobot_SofterIIC.h"

#include "reg.h"
#include "flash.h"
#include "timer.h"

#include "SD3031.h"
#include "L76K.h"


/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void userSetHandle(uint8_t r_address, uint8_t length)
{
  uint8_t rtcWriteReg = 0;
  uint8_t rtcWriteLen = 0;
  printf("r_address = %u, length = %u\r\n", r_address, length);
  while (length--) {
    switch (r_address) {
    case REG_START_GET:
      gps_offset = 0;
      break;
    case REG_GNSS_MODE: {
      uint8_t temp = 0;
      temp = hex_to_ascll_str(regBuf[r_address]);
      set_star_type((char*)&temp);
      break;
    }
    case REG_SLEEP_MODE:
      if (regBuf[r_address] == ENABLE_POWER) {
        enable_gnss_power();
      } else if (regBuf[r_address] == DISABLE_POWER) {
        disable_gnss_power();
      }
      break;
    case REG_RTC_READ_REG: {
      uint8_t reg = regBuf[REG_RTC_READ_REG];   // 要读的寄存器地址
      uint8_t len = regBuf[REG_RTC_READ_LEN];   // 要读的寄存器地址
      if ((reg >= 0x30) && (reg <= 0x79) && (len != 0))
        SD3031readReg(reg - 0x30, &regBuf[reg], len);   // 获取该寄存器最新值
      break;
    }
    case REG_CALIB_RTC_REG:
      if (0 != regBuf[REG_CALIB_RTC_REG]) {   // 置零时, 为禁用自动校时
        regBuf[REG_CALIB_STATUS_REG] = eUnderCalib;
        secTimerFlag = 0;
        minTimerFlag = 0;
      }
      break;
    default:
      if (r_address >= 0x30) {
        if (rtcWriteReg < 0x30) {
          rtcWriteReg = r_address;
        }
        rtcWriteLen++;
      }
      break;
    }
    r_address++;
  }
  if ((rtcWriteLen > 0) && ((rtcWriteReg + rtcWriteLen) <= (0x71 + 1 + 0x30)))   // RTC可写寄存器大致0x00~0x71
    SD3031writeReg(rtcWriteReg - 0x30, &regBuf[rtcWriteReg], rtcWriteLen);   // 将该值写入RTC模块
}

/* Private user code ---------------------------------------------------------*/

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock to HIRC 24MHz*/
  SystemClock_Config();
  // NVIC_SetPriority(SysTick_IRQn, 3);   // 设置更高的滴答优先级，使得它能在一些中断里面使用
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /* Configure uart1 for printf */
  // LogInit();
  // printf("DFRobot GNSS&RTC !\r\n");
  // printf("Printf success using UART1, PD5-TXD, PD6-RXD\r\n");
  // printf("Printf success using UART0, PA2-TXD, PA1-RXD\r\n");

  /* 通信寄存器的初始化 */
  initRegBuf();

  /* GNSS相关引脚初始化 */
  init_l76k_gpio();

  /* uart解析任务的定时器初始化 */
  uartAnalysisTimerInit();

  /* 硬件i2c从机初始化 */
  I2C_Init();

  /* 软件i2c主机初始化 */
  Soft_IIC_Init(0);

  /* SD3031基本初始化初始化 */
  sd3031Init();

  /* uart0 和主控通信 */
  initUart0ForCS32(57600);   // uno软串口，接收115200数据会出现大量错误

  /* uart1 和 L76K 通信 */
  initUart1ForL76K(9600);   // 因为gps传感器默认是9600的波特率
  HAL_Delay(1000);
  set_uart_baud("5");   // 这里更改gps传感器的通信波特率 为115200，为了更快的获取数据
  HAL_Delay(100);
  initUart1ForL76K(115200);

  /* Suspend systemtick to avoid system to be waked up by systemtick */
  // HAL_SuspendTick();
  // 恢复系统滴答定时器
  // HAL_ResumeTick();

  // 每次上电启动时, 获取一次RTC芯片的实时时钟数据
  SD3031readReg(SD3031_REG_SEC, &regBuf[0x30], 7);
  // // 每次上电启动时, 执行一次GNSS校时
  // regBuf[REG_CALIB_STATUS_REG] = eUnderCalib;
  while (1) {
    // i2c 中断 费时处理函数 userSetHandle
    if (userSetFlag == 1) {
      userSetFlag = 0;
      userSetHandle(userSetBeginReg, userSetCount);
    }

    // 定时循环自动校准处理
    if (0 != regBuf[REG_CALIB_RTC_REG]) {
      if ((minTimerFlag / 60) >= regBuf[REG_CALIB_RTC_REG]) {
        regBuf[REG_CALIB_STATUS_REG] = eUnderCalib;
        minTimerFlag = 0;
      }
    }

    // 串口解析
    if (cs32TimerFlag > 5) {// 5ms
      if (cs32RxCount) {
        while ((cs32RxCount - cs32HandleCount) >= 3) {
          annysis_uart0_command();
          HAL_Delay(1);
        }
      } else {
        cs32TimerFlag = 0;
      }
    }

    // gps 串口数据解析
    if (l76kTimerFlag > 5) {// 5ms
      if (l76kRxCount > 0) {
        regBuf[REG_DATA_LEN_H] = l76kRxCount >> 8;
        regBuf[REG_DATA_LEN_L] = l76kRxCount;
        anaysis_gps_data((uint8_t*)l76kUart1Buffer, l76kRxCount);    // 解析数据
        l76kRxCount = 0;
      } else {
        l76kTimerFlag = 0;
      }
    }
    // HAL_PWR_EnterSLEEPMode(PWR_SLEEPENTRY_WFI);   // 低功耗, 等待32u4的i2c访问中断唤醒
    // printf("hello world !\r\n");
    // HAL_Delay(1000);
  }
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
  RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HIRC;
  RCC_OscInitStruct.HIRCState = RCC_HIRC_ON;
  RCC_OscInitStruct.HIRCCalibrationValue = RCC_HIRCCALIBRATION_24M;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /**Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HIRC;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APBCLKDivider = RCC_PCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct) != HAL_OK) {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  printf("Error_Handler !\r\n");
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
     /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */


/* Private function -------------------------------------------------------*/



