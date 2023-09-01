#include "DFRobot_SofterIIC.h"

static uint8_t _addr = 0;

#define Soft_SDA_HIGH()   {HAL_GPIO_WritePin(SOFT_SDA_PORT, SOFT_SDA_PIN, GPIO_PIN_SET);}
#define Soft_SDA_LOW()    {HAL_GPIO_WritePin(SOFT_SDA_PORT, SOFT_SDA_PIN, GPIO_PIN_RESET);}
#define Soft_SCL_HIGH()   {HAL_GPIO_WritePin(SOFT_SCL_PORT, SOFT_SCL_PIN, GPIO_PIN_SET);}
#define Soft_SCL_LOW()    {HAL_GPIO_WritePin(SOFT_SCL_PORT, SOFT_SCL_PIN, GPIO_PIN_RESET);}

uint8_t Soft_Read_SDA(void)
{
  return HAL_GPIO_ReadPin(SOFT_SDA_PORT, SOFT_SDA_PIN);
}

uint8_t Soft_Read_SCL(void)
{
  return HAL_GPIO_ReadPin(SOFT_SCL_PORT, SOFT_SCL_PIN);
}

void SOFT_SCL_OUT()
{
  __HAL_GPIO_SET_OUTPUT(SOFT_SCL_PORT, SOFT_SCL_PIN);
}

void SOFT_SDA_OUT()
{
  __HAL_GPIO_SET_OUTPUT(SOFT_SDA_PORT, SOFT_SDA_PIN);
}

void SOFT_SDA_IN()
{
  __HAL_GPIO_SET_INPUT(SOFT_SDA_PORT, SOFT_SDA_PIN);
}

void Soft_IIC_Delay(void)
{
  __NOP();__NOP();__NOP();__NOP();
  __NOP();__NOP();__NOP();__NOP();
  __NOP();__NOP();__NOP();__NOP();
  __NOP();__NOP();__NOP();__NOP();
  __NOP();__NOP();__NOP();__NOP();
  __NOP();__NOP();__NOP();__NOP();
  __NOP();__NOP();__NOP();__NOP();
  __NOP();__NOP();__NOP();__NOP();
  __NOP();__NOP();__NOP();__NOP();
  __NOP();__NOP();__NOP();__NOP();
  __NOP();__NOP();__NOP();__NOP();
  __NOP();__NOP();__NOP();__NOP();
  __NOP();__NOP();__NOP();__NOP();
  __NOP();__NOP();__NOP();__NOP();
  __NOP();__NOP();__NOP();__NOP();
  __NOP();__NOP();__NOP();__NOP();
  __NOP();__NOP();__NOP();__NOP();

  return;
}

void Soft_IIC_Init(uint8_t addr)
{
  _addr = addr << 1;

  // __HAL_RCC_GPIOD_CLK_ENABLE();
  GPIO_InitTypeDef  softI2C={0};

  softI2C.Pin    = SOFT_SDA_PIN;
  softI2C.Mode = GPIO_MODE_OUTPUT;  
  softI2C.OpenDrain = GPIO_OPENDRAIN; 
  softI2C.Debounce.Enable = GPIO_DEBOUNCE_DISABLE; 
  softI2C.SlewRate = GPIO_SLEW_RATE_HIGH; 
  softI2C.DrvStrength = GPIO_DRV_STRENGTH_HIGH; 
  softI2C.Pull = GPIO_NOPULL;      
  HAL_GPIO_Init(SOFT_SDA_PORT, &softI2C);

  softI2C.Pin = SOFT_SCL_PIN;
  HAL_GPIO_Init(SOFT_SCL_PORT, &softI2C);

  SOFT_SDA_OUT();
  SOFT_SCL_OUT();
  Soft_SDA_HIGH();
  Soft_SCL_HIGH();
}

void Soft_IIC_Start(void)
{
  SOFT_SDA_OUT();
  Soft_SDA_HIGH();
  Soft_SCL_HIGH();
  Soft_IIC_Delay();
  Soft_SDA_LOW();
  Soft_IIC_Delay();
  Soft_SCL_LOW();
}

void Soft_IIC_Stop(void)
{
  SOFT_SDA_OUT();
  Soft_SCL_LOW();
  Soft_SDA_LOW();
  Soft_IIC_Delay();
  Soft_SCL_HIGH();
  Soft_IIC_Delay();
  Soft_SDA_HIGH();
}

void Soft_IIC_Ack(void)
{
  Soft_SCL_LOW();
  SOFT_SDA_OUT();   // 避免SDA切换输出模式时拉高1us，产生NACK，所以先拉低SCL
  Soft_SDA_LOW();
  Soft_IIC_Delay();
  Soft_SCL_HIGH();
  Soft_IIC_Delay();
  Soft_SCL_LOW();
}

void Soft_IIC_NAck(void)
{
  Soft_SCL_LOW();
  SOFT_SDA_OUT();   // 避免SDA切换输出模式时拉高1us，产生错误NACK，所以先拉低SCL
  Soft_SDA_HIGH();
  Soft_IIC_Delay();
  Soft_SCL_HIGH();
  Soft_IIC_Delay();
  Soft_SCL_LOW();
}

uint8_t Soft_IIC_Wait_Ack(void)
{
  uint16_t ErrTime = 0;
  SOFT_SDA_IN();
  Soft_IIC_Delay();
  Soft_SDA_HIGH();
  Soft_SCL_HIGH();
  while(Soft_Read_SDA()){
    ErrTime++;
    if(ErrTime > 100){   // 之前为 250
      Soft_IIC_Stop();
      return 1;
    }
  }
  Soft_IIC_Delay();
  Soft_SCL_LOW();
  Soft_IIC_Delay();
  return 0;
}

void Soft_IIC_WriteByte(uint8_t txd)
{
  uint8_t t;
  SOFT_SDA_OUT();
  for(t = 0; t < 8; t++){
    if((txd & 0x80) >> 7){
      Soft_SDA_HIGH();
    }else{
      Soft_SDA_LOW();
    }
    txd <<= 1;
    Soft_IIC_Delay();
    Soft_SCL_HIGH();
    Soft_IIC_Delay();
    Soft_SCL_LOW();
  }
}

uint8_t Soft_IIC_ReadByte(uint8_t ack)
{
  uint8_t ret = 0;
  SOFT_SDA_IN();
  Soft_SDA_HIGH();
  for(uint8_t i = 0; i < 8; i++){
    Soft_SCL_LOW();
    Soft_IIC_Delay();
    Soft_SCL_HIGH();
    ret <<= 1;
    if(Soft_Read_SDA()){
      ret++;
    }
    Soft_IIC_Delay();
  }
  if(!ack){
    Soft_IIC_NAck();
  }else{
    Soft_IIC_Ack();
  }
  return ret;
}

void setAddr(uint8_t addr)
{
  _addr = addr << 1;
}

void writeReg(uint8_t reg, uint8_t * buf, uint8_t len)
{
  Soft_IIC_Start();
  Soft_IIC_WriteByte(_addr);
  Soft_IIC_Wait_Ack();
  Soft_IIC_WriteByte(reg);
  Soft_IIC_Wait_Ack();
  for(int i=0;i<len;i++){
    Soft_IIC_WriteByte(buf[i]);
    Soft_IIC_Wait_Ack();
  }
  Soft_IIC_Stop();
}

void readReg(uint8_t reg, uint8_t * buf, uint8_t len)
{
  Soft_IIC_Start();
  Soft_IIC_WriteByte(_addr);
  Soft_IIC_Wait_Ack();
  Soft_IIC_WriteByte(reg);
  Soft_IIC_Wait_Ack();
  Soft_IIC_Stop();
  
  Soft_IIC_Start();
  Soft_IIC_WriteByte(_addr|0x01);
  Soft_IIC_Wait_Ack();
  for(int i=0;i<len;i++){
    buf[i]=Soft_IIC_ReadByte(i == (len -1) ? 0 : 1);
  }
  Soft_IIC_Stop();
}

void scan_i2c_address(void)
{
  uint8_t i = 0;
  uint8_t count = 0;
  printf("scaning...\n");

  for(i = 0; i < 127; i++)   // 只扫描写
  {
    Soft_IIC_Start();
    Soft_IIC_WriteByte(i << 1);
    if(0 == Soft_IIC_Wait_Ack())
    {
      printf("i2c find = %#x \n",i);
      count++;
    }
  }
  if(count == 0)
  {
    printf("no find!\n");
  }
  printf("done.\n");
}
