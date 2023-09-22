#include "i2cSlave.h"
#include "L76K.h"
#include "SD3031.h"
#include "reg.h"
#include "uart.h"

I2C_HandleTypeDef i2cSlaveHandle = { 0 };

// uint32_t i2cPrintFlag;
volatile uint8_t userSetFlag = 0;
volatile uint8_t userSetCount = 0;
volatile uint8_t userSetBeginReg = 0;

volatile uint8_t i2c1WrData;
volatile uint8_t i2c1RdData;
volatile uint8_t registerOffset = 0;
volatile bool BeginTransmission = false;

void HAL_I2C_SlaveCallback(I2C_HandleTypeDef* hi2c)
{
  uint32_t i2c_flag = 0XFF;

  HAL_I2C_Wait_Flag(hi2c, &i2c_flag);
  switch (i2c_flag) {
  case I2C_FLAG_SLAVE_RX_SLAW_ACK: // 60H
    userSetCount = 0;
    userSetBeginReg = 0;
    BeginTransmission = true;
    HAL_I2C_ACK_Config(hi2c, ENABLE);
    break;
  case I2C_FLAG_SLAVE_RX_SDATA_ACK: // 80H
    i2c1RdData = hi2c->Instance->DATA;
    if (BeginTransmission) {
      BeginTransmission = false;
      registerOffset = i2c1RdData;   // 第一次写入的值是 寄存器地址
      userSetBeginReg = registerOffset;
    } else {
      if (registerOffset < DATA_LEN_MAX) {   // 防止寄存器溢出
        regBuf[registerOffset] = i2c1RdData;   // 存储写入的数据
        registerOffset++;
        userSetCount++;
      }
    }
    HAL_I2C_ACK_Config(hi2c, ENABLE);
    break;
  case I2C_FLAG_SLAVE_STOP_RESTART:   // 指示从机检测到主机发送的停止或重新启动信号的标志位。
    if (userSetCount != 0) {
      userSetFlag = 1;
    }
    break;
  case I2C_FLAG_SLAVE_TX_SLAW_ACK: // A8H 主机已发送 SLA+W（从机地址加写位）并收到主机 ACK 的标志位。
  case I2C_FLAG_SLAVE_TX_DATA_ACK: // B8H 数据传输期间主机 ACK 从机数据的标志位。
    if ((registerOffset == REG_ALL_DATA) && (gps_offset < UART1_MAX_LEN)) {
      i2c1WrData = l76kUart1Buffer[gps_offset++];
    } else if (registerOffset < DATA_LEN_MAX) {
      i2c1WrData = regBuf[registerOffset];
      if ((REG_CALIB_STATUS_REG == registerOffset) && (eCalibComplete == regBuf[registerOffset])) {
        regBuf[registerOffset] = eCalibNone;   // 消除校准完成的标志位
      }
      registerOffset++;
    } else {
      i2c1WrData = 0xFF;
    }
    HAL_I2C_Send_Byte(hi2c, i2c1WrData);
    break;
  default:
    break;
  }
  // i2cPrintFlag = i2c_flag;
  HAL_I2C_Clear_Interrupt_Flag(hi2c);
}

/**
  * @brief IWDG Init Configuration
  * @retval None
  */
void I2C_Init(void)
{
  /*set init handle*/
  i2cSlaveHandle.Instance = I2C;
  i2cSlaveHandle.Init.master = I2C_MASTER_MODE_DISABLE;   // 主机模式禁止
  i2cSlaveHandle.Init.slave = I2C_SLAVE_MODE_ENABLE;   // 从机模式使能
  i2cSlaveHandle.Mode = HAL_I2C_MODE_SLAVE;   // 从机模式
  i2cSlaveHandle.Init.slaveAddr = CS32_I2C_ADDR;   // 从机地址

  i2cSlaveHandle.Init.broadack = I2C_BROAD_ACK_DISABLE;   // 广播地址应答禁止
  i2cSlaveHandle.Init.speedclock = I2C_SPEED_RATE;   // I2C传输速率
  i2cSlaveHandle.State = HAL_I2C_STATE_RESET;   // I2C状态重置

  HAL_I2C_Init(&i2cSlaveHandle);

  HAL_NVIC_SetPriority(I2C_IRQn, I2C_IRQ_LEVEL);
  HAL_NVIC_EnableIRQ(I2C_IRQn);
}

void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  __HAL_RCC_I2C_CLK_ENABLE();

  /** I2C GPIO Configuration
    PD5 ------> I2C_SCL
    PD6 ------> I2C_SDA
  */
  GPIO_InitTypeDef  gpioi2c = { 0 };
  /* Configure the I2C pin */
  gpioi2c.Pin = GPIO_PIN_6;
  gpioi2c.Mode = GPIO_MODE_AF;   // GPIO端口复用功能 
  gpioi2c.Alternate = GPIO_AF4_I2C_SDA;   // 配置为I2C_SDA
  gpioi2c.OpenDrain = GPIO_OPENDRAIN;   // 开漏输出
  gpioi2c.Debounce.Enable = GPIO_DEBOUNCE_DISABLE;   // 禁止输入去抖动
  gpioi2c.SlewRate = GPIO_SLEW_RATE_HIGH;   // 电压转换速率
  gpioi2c.DrvStrength = GPIO_DRV_STRENGTH_HIGH;   // 驱动强度
  gpioi2c.Pull = GPIO_PULLUP;   // 上拉
  HAL_GPIO_Init(GPIOD, &gpioi2c);

  gpioi2c.Pin = GPIO_PIN_5;
  gpioi2c.Alternate = GPIO_AF4_I2C_SCL;
  gpioi2c.SlewRate = GPIO_SLEW_RATE_LOW;
  gpioi2c.DrvStrength = GPIO_DRV_STRENGTH_LOW;
  HAL_GPIO_Init(GPIOD, &gpioi2c);
}

void slave_i2c_test(void)
{
  // if (0xFF != i2cPrintFlag) {
  //   printf("flag = %#x\r\n", i2cPrintFlag);
  //   printf("regBuf[0] = %#x\r\n", regBuf[0]);
  //   printf("regBuf[1] = %#x\r\n", regBuf[1]);
  //   printf("regBuf[2] = %#x\r\n", regBuf[2]);
  //   i2cPrintFlag = 0xFF;
  // }
}
