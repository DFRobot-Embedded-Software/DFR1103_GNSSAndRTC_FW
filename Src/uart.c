#include "uart.h"
#include "timer.h"
#include "reg.h"
#include "L76K.h"

UART_HandleTypeDef huart0 = { 0 };   // For CS32
UART_HandleTypeDef huart1 = { 0 };   // For L76K
// log.c打印使用的句柄, 定义在log.c中, 打印必定会卡死, 原因暂时未知, 所以定义在此处
UART_HandleTypeDef customHuart = { 0 };

uint8_t cs32Uart0Buffer[UART0_MAX_LEN] = { 0 }, l76kUart1Buffer[UART1_MAX_LEN] = { 0 };
volatile uint8_t cs32RxCount = 0, cs32HandleCount = 0;
volatile uint16_t l76kRxCount = 0;
static uint8_t cs32RxData = 0, l76kRxData = 0;

/**
 * @brief 串口数据解析
 * @details 串口数据分为如下读写两类:
 * @n 写操作 :
 * @n 发送 : UART0_WRITE_REGBUF + 寄存器地址 + 写入数据长度 + 对应长度的数据字节
 * @n 读操作 :
 * @n 如果是读取RTC模块的数据，需要先将(寄存器地址 + 读取数据长度) 写入(通过写操作) (REG_RTC_READ_REG + REG_RTC_READ_LEN)，以更新对应寄存器数据
 * @n 发送 : UART0_READ_REGBUF + 寄存器地址 + 读取数据长度 ; 接收 : 读取长度的字节
 */
void annysis_uart0_command(void)
{
  uint8_t* data = &cs32Uart0Buffer[cs32HandleCount];
  uint8_t type = data[0];
  uint8_t reg = data[1];
  uint8_t len = data[2];
  switch (type) {
  case UART0_READ_REGBUF: {
    if ((cs32RxCount - cs32HandleCount) >= 3) {   // 防止uart数据不完整
      if (reg == REG_ALL_DATA) {
        uart0Send((uint8_t*)(&l76kUart1Buffer[gps_offset]), len);
        gps_offset += len;
      } else if ((reg + len) <= DATA_LEN_MAX) {
        uart0Send(&regBuf[reg], len);
        if ((REG_CALIB_STATUS_REG >= reg) && (REG_CALIB_STATUS_REG <= (reg + len - 1)) \
          && (eCalibComplete == regBuf[REG_CALIB_STATUS_REG])) {
          regBuf[REG_CALIB_STATUS_REG] = eCalibNone;   // 消除校准完成的标志位
        }
      }
      cs32HandleCount += 3;
    }
    break;
  }
  case UART0_WRITE_REGBUF:
    if (cs32RxCount - cs32HandleCount >= (3 + len)) {   // 防止uart数据不完整
      memcpy(&regBuf[reg], &data[3], len);
      userSetHandle(reg, len);
      cs32HandleCount += (3 + len);
    }
    break;
  default:
    break;
  }

  if (cs32HandleCount == cs32RxCount) {
    cs32RxCount = 0;
    cs32HandleCount = 0;
  }
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  huart: pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
  if (&huart0 == huart) {
    if (cs32RxCount < UART0_MAX_LEN) {
      cs32Uart0Buffer[cs32RxCount++] = cs32RxData;
      cs32RxCount %= UART0_MAX_LEN;
    }
    cs32TimerFlag = 0;
    HAL_UART_Receive_IT(&huart0, (uint8_t*)&cs32RxData, 1);
  } else if (&huart1 == huart) {
    // 防止数据溢出
    if (l76kRxCount < UART1_MAX_LEN) {
      l76kUart1Buffer[l76kRxCount++] = l76kRxData;
    }
    l76kTimerFlag = 0;         // 计数器标志清零
    HAL_UART_Receive_IT(&huart1, (uint8_t*)&l76kRxData, 1);
  }
}

void uart0Send(uint8_t* data, uint8_t len)
{
  HAL_UART_Transmit_IT(&huart0, data, len);
}

void uart1Send(uint8_t* data, uint8_t len)
{
  HAL_UART_Transmit_IT(&huart1, data, len);
}

void initUart0ForCS32(uint32_t baud)
{
  /* Peripheral clock enable */
  __HAL_RCC_UART0_CLK_ENABLE();

  huart0.Instance = UART0;   // UART0
  huart0.Init.BaudRate = baud;   // 波特率
  huart0.Init.BaudDouble = UART_BAUDDOUBLE_ENABLE;   // 双波特率禁用 : UART_BAUDDOUBLE_DISABLE
  huart0.Init.WordLength = UART_WORDLENGTH_8B;   // 数据长度
  huart0.Init.Parity = UART_PARITY_NONE;   // 无校验
  huart0.Init.Mode = UART_MODE_TX_RX;   // 接收和发送使能
  // huart0.Init.StopBits = UART_STOPBITS_1;
  // huart0.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  // huart0.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart0) != HAL_OK) {
    Error_Handler();
  }

  HAL_UART_Receive_IT(&huart0, &cs32RxData, sizeof(cs32RxData));
  HAL_NVIC_SetPriority(UART0_IRQn, 1);   // 
  HAL_NVIC_EnableIRQ(UART0_IRQn);   // 中断使能

}

void initUart1ForL76K(uint32_t baud)
{
  /* Peripheral clock enable */
  __HAL_RCC_UART1_CLK_ENABLE();

  huart1.Instance = UART1;   // UART1
  huart1.Init.BaudRate = baud;   // 波特率
  huart1.Init.BaudDouble = UART_BAUDDOUBLE_ENABLE;   // 双波特率启用 : UART_BAUDDOUBLE_ENABLE
  huart1.Init.WordLength = UART_WORDLENGTH_8B;   // 数据长度
  huart1.Init.Parity = UART_PARITY_NONE;   // 无校验
  huart1.Init.Mode = UART_MODE_TX_RX;   // 接收和发送使能
  if (HAL_UART_Init(&huart1) != HAL_OK) {
    Error_Handler();
  }

  HAL_UART_Receive_IT(&huart1, &l76kRxData, sizeof(l76kRxData));
  HAL_NVIC_SetPriority(UART1_IRQn, 1);
  HAL_NVIC_EnableIRQ(UART1_IRQn);   // 中断使能

}

/**
  * @brief  UART MSP Init.
  * @param  uartHandle: pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  if (uartHandle->Instance == UART0) {
    /* GPIO clock enable */
    // __HAL_RCC_GPIOA_CLK_ENABLE();

    /** USART0 GPIO Configuration
     * PA1 ------> UART0_RXD
     * PA2 ------> UART0_TXD
    */
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    GPIO_InitStruct.Pin = GPIO_PIN_1;   // GPIO选择
    GPIO_InitStruct.Mode = GPIO_MODE_AF;   // 复用功能
    GPIO_InitStruct.OpenDrain = GPIO_PUSHPULL;   // 推挽输出
    GPIO_InitStruct.Debounce.Enable = GPIO_DEBOUNCE_DISABLE;   // 禁止输入去抖动
    GPIO_InitStruct.SlewRate = GPIO_SLEW_RATE_HIGH;   // 电压转换速率
    GPIO_InitStruct.DrvStrength = GPIO_DRV_STRENGTH_HIGH;   // 驱动强度
    GPIO_InitStruct.Pull = GPIO_PULLUP;   // 上拉
    GPIO_InitStruct.Alternate = GPIO_AF5_UART0_RXD;    // 端口复用
    // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF;
    GPIO_InitStruct.Alternate = GPIO_AF5_UART0_TXD;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  } else if (uartHandle->Instance == UART1) {
    /* GPIO clock enable */
    // __HAL_RCC_GPIOC_CLK_ENABLE();

    /**
     * USART1 GPIO Configuration
     * PC3 ------> UART1_TXD
     * PC4 ------> UART1_RXD
     */
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF;
    GPIO_InitStruct.OpenDrain = GPIO_PUSHPULL;
    GPIO_InitStruct.Debounce.Enable = GPIO_DEBOUNCE_DISABLE;
    GPIO_InitStruct.SlewRate = GPIO_SLEW_RATE_HIGH;
    GPIO_InitStruct.DrvStrength = GPIO_DRV_STRENGTH_HIGH;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Alternate = GPIO_AF5_UART1_RXD;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF;
    GPIO_InitStruct.Alternate = GPIO_AF5_UART1_TXD;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
  if (uartHandle->Instance == UART0) {
    /* Peripheral clock disable */
    __HAL_RCC_UART0_CLK_DISABLE();

    /**
     * USART0 GPIO Configuration
     * PA1     ------> UART0_RXD
     * PA2     ------> UART0_TXD
     */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1 | GPIO_PIN_2);

  } else if (uartHandle->Instance == UART1) {
    /* Peripheral clock disable */
    __HAL_RCC_UART1_CLK_DISABLE();

    /**
     * USART1 GPIO Configuration
     * PC3 ------> UART1_TXD
     * PC4 ------> UART1_RXD
     */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_3 | GPIO_PIN_4);
  }
}
