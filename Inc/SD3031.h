/*!
 * @file DFRobot_SD3031.h
 * @brief Define the basic structure of class DFRobot_SD3031
 * @details Define SD3031 functions
 * @copyright	Copyright (c) 2021 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author [TangJie](jie.tang@dfrobot.com)
 * @version V1.0
 * @date 2022-07-25
 * @url https://github.com/DFRobot/DFRobot_SD3031
 */
#ifndef _SD3031_H_
#define _SD3031_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

    /* Macro */
#define SD3031_IIC_ADDRESS        0x32  ///< Sensor device address

#define SD3031_REG_SEC            0x00  ///< RTC Seconds Register
#define SD3031_REG_MIN            0x01  ///< RTC Minutes Register
#define SD3031_REG_HOUR           0x02  ///< RTC Hours Register
#define SD3031_REG_WEEK           0x03  ///< RTC Week Register
#define SD3031_REG_DAY            0x04  ///< RTC Day Register
#define SD3031_REG_MONTE          0x05  ///< RTC Month Register
#define SD3031_REG_YEAR           0x06  ///< RTC Year Register
#define SD3031_REG_ALARM_SEC      0x07  ///< RTC Seconds Alarm Register
#define SD3031_REG_ALARM_MIN      0x08  ///< RTC Minutes Alarm Register
#define SD3031_REG_ALARM_HOUR     0x09  ///< RTC Hours Alarm Register
#define SD3031_REG_ALARM_WEEK     0x0A  ///< RTC Week Alarm Register
#define SD3031_REG_ALARM_DAY      0x0B  ///< RTC Day Alarm Register
#define SD3031_REG_ALARM_MONNTE   0x0C  ///< RTC Month Alarm Register
#define SD3031_REG_ALARM_YEAR     0x0D  ///< RTC Year Alarm Register
#define SD3031_REG_ALARM_CON      0x0E  ///< RTC Alarm Control Register
#define SD3031_REG_CTR1           0x0F  ///< Control Register 1
#define SD3031_REG_CTR2           0x10  ///< Control Register 2
#define SD3031_REG_CTR3           0x11  ///< Control Register 3
#define SD3031_REG_COUNTDOWM      0X13  ///< Countdown Register
#define SD3031_REG_TEMP           0x16  ///< Internal Temperature Register
#define SD3031_REG_IIC_CON        0x17  ///< I2C Control
#define SD3031_REG_BAT_VAL        0x1A  ///< Battery Level

    /**
     * @struct sTimeData_t
     * @brief Structure for storing time data
     */
    typedef struct {
        uint16_t year;
        uint8_t  month;
        uint8_t  day;
        uint8_t  week;   // 修改 String 类型
        uint8_t  hour;
        uint8_t minute;
        uint8_t second;
    }sTimeData_t;

    /**
     * @enum  eHours_t
     * @brief  e24hours, e12hours
     */
    typedef enum {
        e12hours = 0,
        e24hours = 1 << 7,
    }eHours_t;

    /**
     * @enum eWeek_t
     * @brief Enumerate week definition
     */
    typedef enum {
        eSunday = 0x01,
        eMonday = 0x02,
        eTuesday = 0x04,
        eWednesday = 0x08,
        eThursday = 0x10,
        eFriday = 0x20,
        eSaturday = 0x40,
        eEveryDay = 0x7f,
        eWorkday = 0x3e,
    }eWeek_t;


    /**
     * @enum  eTrigger_t
     * @brief  Enumerate interrupt definition
     */
    typedef enum {
        eYearEnable = 0x40,
        eMondayEnable = 0x20,
        eDayEnable = 0x10,
        eHoursEnable = 0x04,
        eMinuteEnable = 0x02,
        eSecondEnable = 0x01,
    }eTrigger_t;

    uint8_t SD3031readReg(uint8_t reg, void* pBuf, size_t size);
    uint8_t SD3031writeReg(uint8_t reg, void* pBuf, size_t size);

    /* Function */
    void sd3031Init(void);

    /**
     * @fn getRTCTime
     * @brief 获取时钟模块中的年
     * @return 返回获取的年份
     */
    sTimeData_t getRTCTime(void);

    /**
     * @brief 设置时钟是24小时制还是12小时制
     * @param mode 时钟计算方式
     */
    void setHourSystem(eHours_t mode);

    /**
     * @fn setTime
     * @brief 设置时钟时间
     * @param year 2000~2099
     * @param month 1~12
     * @param day 1~31
     * @param hour 0~23
     * @param minute 0~59
     * @param second 0~59
     * @return None
     */
    void setTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);

    /**
     * @fn setAlarmYMD
     * @brief Set the data for triggering alarm
     * @param year 2000~2099
     * @param month 1~12
     * @param day 1~31
     * @return None
     */
    void setAlarmYMD(uint16_t year, uint8_t month, uint8_t day);

    /**
     * @fn setAlarmWHMS
     * @brief Set the Alarmnumber object
     * @param week
     * @n ---------------------------------------------------------------------------------------------------------
     * @n |    bit7    |    bit6    |    bit5    |    bit4    |    bit3    |    bit2    |    bit1    |    bit0    |
     * @n ---------------------------------------------------------------------------------------------------------
     * @n |            |  Saturday  |  Friday    |  Thursday  | Wednesday  |  Tuesday   |  Monday    |  Sunday    |
     * @n ---------------------------------------------------------------------------------------------------------
     * @param hour 0~23
     * @param minute 0~59
     * @param second 0~59
     */
    void setAlarmWHMS(uint8_t week, uint8_t hour, uint8_t minute, uint8_t second);

    /**
     * @brief 获取时钟内部温度
     * @return 返回获取得温度，单位：℃
     */
    int8_t getTemperatureC(void);

    /**
     * @brief 获取板载电池电压
     * @return float 返回获取得电压
     */
    float getVoltage(void);

    /**
     * @brief 清除报警标志位
     */
    void clearAlarm(void);
    /**
     * @fn getAMorPM
     * @brief 输出上午或下午的时间
     * @return 上午或下午的时间，24小时模式返回空字符串
     */
     // String getAMorPM();

     /**
      * @fn enable32k
      * @brief 开启32k频率输出
      * @return 无
      */
    void enable32k(void);

    /**
     * @fn disable32k
     * @brief 关闭32k输出
     * @return 无
     */
    void disable32k(void);
    /**
     * @fn writeSRAM
     * @brief 写 SRAM
     * @param addr 0x14~0xFF
     * @param data 写数据
     * @return true 意味着写SRAM是成功的, false 意味着写SRAM是失败的
     */
    uint8_t writeSRAM(uint8_t addr, uint8_t data);

    /**
     * @fn readSRAM
     * @brief 读 SRAM
     * @param addr 0x14~0xFF
     * @return 存储在SRAM中的数据
     */
    uint8_t readSRAM(uint8_t addr);

    /**
     * @fn clearSRAM
     * @brief 清除SRAM
     * @param addr 0x14~0xFF
     * @return true 意味着清除SRAM是成功的, false 意味着清除SRAM是失败的
     */
    uint8_t clearSRAM(uint8_t addr);

    /**
     * @fn countDown
     * @brief 倒计时
     * @param second  倒计时时间 0~0xffffff
     */
    void countDown(uint32_t second);

    /**
     * @fn bcd2bin(uint8_t val)
     * @brief BCD code to BIN code
     * @param val Input BCD code
     * @return Return BIN code
     */
    uint8_t bcd2bin(uint8_t val);

    /**
     * @fn bin2bcd(uint8_t val)
     * @brief BIN code to BCD code
     * @param val Input BIN code
     * @return Return BCD code
     */
    uint8_t bin2bcd(uint8_t val);

#ifdef __cplusplus
}
#endif

#endif /* _SD3031_H_ */
