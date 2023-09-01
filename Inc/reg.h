#ifndef __REG_H__
#define __REG_H__
#include "main.h"


#define	DATA_LEN_MAX     0xB0   // 寄存器buf长度

// GNSS (L76K) 寄存器地址 0x00 ~ 0x24
#define REG_YEAR_H       0
#define REG_YEAR_L       1
#define REG_MONTH        2
#define REG_DATE         3
#define REG_HOUR         4
#define REG_MINUTE       5
#define REG_SECOND       6

#define REG_LAT_1        7
#define REG_LAT_2        8
#define REG_LAT_X_24     9
#define REG_LAT_X_16     10
#define REG_LAT_X_8      11
#define REG_LAT_DIS      12

#define REG_LON_1        13
#define REG_LON_2        14
#define REG_LON_X_24     15
#define REG_LON_X_16     16
#define REG_LON_X_8      17
#define REG_LON_DIS      18

#define REG_USE_STAR     19

#define REG_ALT_H        20
#define REG_ALT_L        21
#define REG_ALT_X        22

#define REG_SOG_H        23
#define REG_SOG_L        24
#define REG_SOG_X        25

#define REG_COG_H        26
#define REG_COG_L        27
#define REG_COG_X        28


#define REG_START_GET    29
#define REG_I2C_ADDR     30
#define REG_DATA_LEN_H   31
#define REG_DATA_LEN_L   32
#define REG_ALL_DATA     33

#define REG_GNSS_MODE    34
#define REG_SLEEP_MODE   35
// #define REG_RGB_MODE     36   // 0x24

// RTC (SD3031) 寄存器地址 0x30(0x00 + 0x30) ~ 0xA9(0x79 + 0x30)

// 自定义功能寄存器 0x24 ~ 0x2F
#define REG_CALIB_STATUS_REG   0x2AU
#define REG_CALIB_RTC_REG      0x2BU
// #define REG_RTC_WRITE_REG      0x2CU
// #define REG_RTC_WRITE_LEN      0x2DU
#define REG_RTC_READ_REG       0x2EU
#define REG_RTC_READ_LEN       0x2FU

// 自定义功能寄存器 0xAA ~ (0xB0 - 1)
#define REG_CS32_PID           0xAAU
#define REG_CS32_VID           0xACU
#define REG_CS32_VERSION       0xAEU


extern uint8_t regBuf[DATA_LEN_MAX];

void initRegBuf(void);

#endif/* __REG_H__ */
