#include "reg.h"
#include "i2cSlave.h"

uint8_t regBuf[DATA_LEN_MAX] = { 0 };

void initRegBuf(void)
{
  /* cs32 作为从机的寄存器初始化 */
  //read data from flash
  // MY_FLASH_read(&regBuf[CS32_I2C_PID_REG], 8);

  // if (regBuf[CS32_I2C_PID_REG] != 0xAA) {   // 判断flash中是否存有有效值
  //   //!< PID: 模块的PID (DFR0682)(最高两位作为种类判断00:SEN、01:DFR、10:TEL、11:BOS, 后面14位作为num)
  //   regBuf[CS32_I2C_PID_REG + 1] = 0x42;   // MSB
  //   regBuf[CS32_I2C_PID_REG] = 0xAA;       // LSB
  //   //!< VID: DFRobot
  //   regBuf[CS32_I2C_VID_REG + 1] = 0x33;
  //   regBuf[CS32_I2C_VID_REG] = 0x43;
  //   //!< 固件版本: 0.1.0.0
  //   regBuf[CS32_I2C_VERSION_REG + 1] = 0x01;
  //   regBuf[CS32_I2C_VERSION_REG] = 0x00;

  //   MY_FLASH_write(&regBuf[CS32_I2C_PID_REG], 8);
  // }
  regBuf[REG_I2C_ADDR] = CS32_I2C_ADDR;

  //!< PID: 模块的PID (DFR1103)(最高两位作为种类判断00:SEN、01:DFR、10:TEL、11:BOS, 后面14位作为num)
  regBuf[REG_CS32_PID + 1] = 0x44;   // MSB
  regBuf[REG_CS32_PID] = 0x4F;       // LSB
  //!< VID: DFRobot
  regBuf[REG_CS32_VID + 1] = 0x33;
  regBuf[REG_CS32_VID] = 0x43;
  //!< 固件版本: 0.1.0.0
  regBuf[REG_CS32_VERSION + 1] = 0x01;
  regBuf[REG_CS32_VERSION] = 0x00;

}
