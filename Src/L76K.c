#include "L76K.h"
#include "reg.h"
#include "uart.h"
#include "SD3031.h"

s_gga gngga_data;
s_vtg gnvtg_data;
s_zda gnzda_data;
s_gps my_gps_data;
s_utc my_utc_data;

uint16_t gps_offset = 0;


void enable_gnss_power(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);   // 使能gps电源 VCC
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);     // 拉高RESET
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);     // 拉高wake up
    // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);   // 悬空：BeiDou + GPS；低电平：GPS + GLONASS。
}

// extern UART_HandleTypeDef huart1;
void disable_gnss_power(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);     // 失能gps电源 VCC
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);   // 拉低RESET
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);   // 拉低wake up
    // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);   // 悬空：BeiDou + GPS；低电平：GPS + GLONASS。
}

/**
 * @brief 设置gnss 和cs 32连接的gpio
 * @n  同时 cs32 用 PA3 控制 L76K_PWR; 用 PB5 控制 L76K_RST;
 * @n  用 PB4 控制 L76K_WAKEUP; 用 PC5 控制 L76K_SET。
 */
void init_l76k_gpio(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT;
    GPIO_InitStruct.OpenDrain = GPIO_PUSHPULL;
    GPIO_InitStruct.Debounce.Enable = GPIO_DEBOUNCE_DISABLE;   // 禁止输入去抖动
    GPIO_InitStruct.SlewRate = GPIO_SLEW_RATE_HIGH;   // 电压转换速率
    GPIO_InitStruct.DrvStrength = GPIO_DRV_STRENGTH_HIGH;   // 驱动强度
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);   // PA3 控制 L76K_PWR

    GPIO_InitStruct.Pin = GPIO_PIN_4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);   // PB4 控制 L76K_WAKEUP

    GPIO_InitStruct.Pin = GPIO_PIN_5;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);   // PB5 控制 L76K_RST

    // GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    // HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);   // PC5 控制 L76K_SET

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);   // 使能gps电源
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);     // 拉高RESET
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);     // 拉高wake up
    // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);   // 悬空：BeiDou + GPS；低电平：GPS + GLONASS。
}

/**
 * @brief hex 转字符串
 * @param data 需要转换的数据
 * @return 转换后的数据
 * @retval  0-9 转为 字符串的0-9
 * @retval  A-F 转为 字符串的A-F
 * @retval  其他数据不转换
 */
uint8_t hex_to_ascll_str(uint8_t data)
{
    if (data <= 0x09) { // range 0x00 0x09
        return 0x30 + data;
    } else if (data >= (uint8_t)0x0A && data <= (uint8_t)0x0F) {
        return 0x41 + data - 0x0A;
    } else {
        return data;
    }
}

/**
 * @brief 重启传感器
 *
 * @param mode 0-3
 *      重启模式：
 *      0 = 热启动
 *      1 = 温启动
 *      2 = 冷启动
 *      3 = 冷启动并恢复出厂设置
 * @return null
 */
void set_reset_mode(char* mode)
{
    char* str = "$PCAS10,";
    uint8_t temp[100] = { 0 };
    uint8_t checknum = 0;
    uint8_t str_len = 0;
    memcpy(temp, str, strlen(str));

    str_len = strlen(str) + 1;

    if (mode[0] >= '0' && mode[0] <= '3') {
        temp[8] = mode[0];
    } else {
        return;
    }
    for (uint8_t i = 1; i <= str_len; i++) {
        checknum ^= temp[i];
    }

    temp[str_len++] = '*';
    temp[str_len++] = hex_to_ascll_str((checknum & 0xF0) >> 4);
    temp[str_len++] = hex_to_ascll_str((checknum & 0x0F));
    temp[str_len++] = 0x0D;
    temp[str_len++] = 0x0A;

    uart1Send(temp, str_len);
}

/**
 * @brief 设置刷新频率
 */
void set_frequency(char* frequency)
{
    char* str = "$PCAS02,";
    uint8_t temp[100] = { 0 };
    uint8_t str_len = 0;
    uint8_t checknum = 0;
    memcpy(temp, str, strlen(str));

    str_len = strlen(frequency) + strlen(str);
    if (strcmp(frequency, "1000") == 0) {
        memcpy(&temp[8], frequency, strlen(frequency));
    } else if (strcmp(frequency, "500") == 0) {
        memcpy(&temp[8], frequency, strlen(frequency));
    } else if (strcmp(frequency, "250") == 0) {
        memcpy(&temp[8], frequency, strlen(frequency));
    } else if (strcmp(frequency, "200") == 0) {
        memcpy(&temp[8], frequency, strlen(frequency));
    } else {
        return;
    }

    for (uint8_t i = 1; i <= str_len; i++) {
        checknum ^= temp[i];
    }
    temp[str_len++] = '*';
    temp[str_len++] = hex_to_ascll_str((checknum & 0xF0) >> 4);
    temp[str_len++] = hex_to_ascll_str((checknum & 0x0F));
    temp[str_len++] = 0x0D;
    temp[str_len++] = 0x0A;
    uart1Send(temp, str_len);
}

/**
 * @brief L76K 串口波特率配置
 *
 * @param baud 0 - 5
 *              0 = 4800
 *              1 = 9600
 *              2 = 19200
 *              3 = 38400
 *              4 = 57600
 *              5 = 115200
 * @return null
 */
void set_uart_baud(char* baud)
{
    char* str = "$PCAS01,";
    uint8_t temp[100] = { 0 };
    uint8_t checknum = 0;
    uint8_t str_len = 0;
    memcpy(temp, str, strlen(str));
    str_len = strlen(str) + 1;

    if (baud[0] >= '0' && baud[0] <= '5') {
        temp[8] = baud[0];
    } else {
        // printf("%x \n",baud[0]);
        return;
    }

    for (uint8_t i = 1; i < str_len; i++) {
        checknum = temp[i] ^ checknum;
    }
    temp[str_len++] = '*';
    temp[str_len++] = hex_to_ascll_str((checknum & 0xF0) >> 4);
    temp[str_len++] = hex_to_ascll_str((checknum & 0x0F));
    temp[str_len++] = 0x0D;
    temp[str_len++] = 0x0A;
    uart1Send(temp, str_len);
}

/**
 * @brief 配置 GNSS 星系
 * @param type 星系
 *          1 = GPS
 *          2 = BeiDou
 *          3 = GPS + BeiDou
 *          4 = GLONASS
 *          5 = GPS + GLONASS
 *          6 = BeiDou + GLONASS
 *          7 = GPS + BeiDou + GLONASS
 * @return null
 */
void set_star_type(char* type)
{
    char* str = "$PCAS04,";
    uint8_t temp[100] = { 0 };
    uint8_t checknum = 0;
    uint8_t str_len = 0;
    memcpy(temp, str, strlen(str));

    str_len = strlen(str) + 1;

    if (type[0] >= '0' && type[0] <= '7') {
        temp[8] = type[0];
    } else {
        return;
    }

    for (uint8_t i = 1; i < str_len; i++) {
        checknum = temp[i] ^ checknum;
    }
    temp[str_len++] = '*';
    temp[str_len++] = hex_to_ascll_str((checknum & 0xF0) >> 4);
    temp[str_len++] = hex_to_ascll_str((checknum & 0x0F));
    temp[str_len++] = 0x0D;
    temp[str_len++] = 0x0A;
    uart1Send(temp, str_len);
}

/**
 * @brief gps数据解析
 *
 * @param data gps数据
 * @param datalen gps数据长度
 * @note  完整的gps数据 数据以'0D 0A'结尾
 * @n $GNGGA,,,,,,0,00,25.5,,,,,,*64                           解析
 * @n $GNGLL,,,,,021637.000,V,N*65
 * @n $GNGSA,A,1,,,,,,,,,,,,,13.4,8.2,10.6,1*38\r\n
 * @n $GNGSA,A,1,,,,,,,,,,,,,13.4,8.2,10.6,4*3D\r\n
 * @n $GPGSV,3,1,11,02,72,130,,05,35,062,,10,06,279,,13,42,042,,0*6C\r\n
 * @n $GNGSA,A,1,,,,,,,,,,,,,13.4,8.2,10.6,2*3B\r\n
 * @n $GPGSV,3,2,11,15,74,022,,18,54,325,,20,07,088,,23,30,295,,0*66\r\n
 * @n $GPGSV,3,3,11,24,32,157,28,29,40,217,,195,35,149,25,0*60\r\n
 * @n $BDGSV,2,1,05,06,54,173,,09,39,185,,10,09,197,,30,19,165,31,0*73\r\n
 * @n $BDGSV,2,2,05,37,14,087,20,0*4D\r\n
 * @n $GLGSV,2,1,07,74,08,078,,66,20,250,,86,,,27,65,57,310,,0*44\r\n
 * @n $GLGSV,2,2,07,88,55,321,,87,69,176,,72,34,026,,0*48\r\n
 * @n $GNRMC,021637.000,V,,,,,,,251022,,,N,V*2E\r\n
 * @n $GNVTG,,,,,,,,,N*2E\r\n                                   解析
 * @n $GNZDA,021637.000,25,10,2022,00,00*4D\r\n                 解析
 * @n $GPTXT,01,01,01,ANTENNA OK*35\r\n
 */
uint8_t* p_data[30] = { 0 };
uint8_t ss_data[30][15] = { 0 };
uint8_t ss_data_len[30] = { 0 };
void anaysis_gps_data(uint8_t* data, uint16_t datalen)
{
    uint8_t talkID = 0;
    uint32_t xiaoshu;

    uint8_t list_num = 0;
    // 防止越界 所以datalen - 1
    p_data[0] = data;
    for (uint16_t i = 0; i < datalen - 1; i++) {
        if (data[i] == 0x0D && data[i + 1] == 0x0A) {
            data[i + 1] = '\0';        // 为了分包,  主机读到'\0' 转成 '\n' 不改变原始数据
            list_num++;
            if ((i + 2) < datalen) {  // 防止越界
                p_data[list_num] = &data[i + 2];  // 数组指针指向下一个位置
            }
        }
    }

    uint8_t list_count = 0;
    for (uint8_t list_number = 0; list_number < list_num; list_number++) {
        uint8_t list_len = strlen((const char*)(p_data[list_number]));       // 每条命令的长度
        uint8_t douhao_temp = 0;
        // 解析ZDA数据
        if (strncmp((const char*)(&p_data[list_number][3]), ZDA, 4) == 0) {
            list_count = 0;
            memset(ss_data, 0, sizeof(ss_data));
            for (uint8_t i = 0; i < list_len; i++) {
                if (p_data[list_number][i] == ',') {
                    ss_data_len[list_count] = i - douhao_temp;
                    memcpy(&ss_data[list_count++][0], &p_data[list_number][douhao_temp], i - douhao_temp);
                    douhao_temp = i + 1;
                }
            }
            memset((void*)&gnzda_data, 0, sizeof(gnzda_data));
            memcpy(gnzda_data.TalkerID, &ss_data[0][0], ss_data_len[0]);
            memcpy(gnzda_data.utc, &ss_data[1][0], ss_data_len[1]);
            memcpy(gnzda_data.day, &ss_data[2][0], ss_data_len[2]);
            memcpy(gnzda_data.month, &ss_data[3][0], ss_data_len[3]);
            memcpy(gnzda_data.year, &ss_data[4][0], ss_data_len[4]);

            memset((void*)&my_utc_data, 0, sizeof(my_utc_data));

            memcpy(my_utc_data.hour, gnzda_data.utc, 2);
            memcpy(my_utc_data.minute, gnzda_data.utc + 2, 2);
            memcpy(my_utc_data.second, gnzda_data.utc + 4, 2);

            my_gps_data.year = atoi(gnzda_data.year);
            my_gps_data.month = atoi(gnzda_data.month);
            my_gps_data.date = atoi(gnzda_data.day);

            my_gps_data.hour = atoi((const char*)my_utc_data.hour);
            my_gps_data.minute = atoi((const char*)my_utc_data.minute);
            my_gps_data.second = atoi((const char*)my_utc_data.second);

            regBuf[REG_YEAR_H] = my_gps_data.year >> 8;
            regBuf[REG_YEAR_L] = my_gps_data.year;
            regBuf[REG_MONTH] = my_gps_data.month;
            regBuf[REG_DATE] = my_gps_data.date;
            regBuf[REG_HOUR] = my_gps_data.hour;
            regBuf[REG_MINUTE] = my_gps_data.minute;
            regBuf[REG_SECOND] = my_gps_data.second;
            // printf("%d/%d/%d/%d:%d:%d\r\n", my_gps_data.year, my_gps_data.month, my_gps_data.date, my_gps_data.hour, my_gps_data.minute, my_gps_data.second);
        }

        //解析GGA数据
        if (strncmp((const char*)(&p_data[list_number][3]), GGA, 4) == 0) {
            memset(ss_data, 0, sizeof(ss_data));
            for (uint8_t i = 0; i < list_len; i++) {
                if (p_data[list_number][i] == ',') {
                    ss_data_len[list_count] = i - douhao_temp;
                    memcpy(&ss_data[list_count++][0], &p_data[list_number][douhao_temp], i - douhao_temp);
                    douhao_temp = i + 1;
                }
            }
            memset((void*)&gngga_data, 0, sizeof(gngga_data));
            memcpy(gngga_data.TalkerID, &ss_data[0][0], ss_data_len[0]);
            memcpy(gngga_data.utc, &ss_data[1][0], ss_data_len[1]);
            memcpy(gngga_data.lat, &ss_data[2][0], ss_data_len[2]);
            memcpy(gngga_data.ns, &ss_data[3][0], ss_data_len[3]);
            memcpy(gngga_data.lon, &ss_data[4][0], ss_data_len[4]);
            memcpy(gngga_data.ew, &ss_data[5][0], ss_data_len[5]);
            memcpy(gngga_data.Quality, &ss_data[6][0], ss_data_len[6]);
            memcpy(gngga_data.NumSatUsed, &ss_data[7][0], ss_data_len[7]);
            memcpy(gngga_data.HDOP, &ss_data[8][0], ss_data_len[8]);
            memcpy(gngga_data.alt, &ss_data[9][0], ss_data_len[9]);

            my_gps_data.lat = atof(gngga_data.lat);
            my_gps_data.lon = atof(gngga_data.lon);
            my_gps_data.ew = gngga_data.ew[0];
            my_gps_data.ns = gngga_data.ns[0];
            my_gps_data.numSatUsed = atoi(gngga_data.NumSatUsed);
            my_gps_data.alt = atof(gngga_data.alt);

            regBuf[REG_USE_STAR] = my_gps_data.numSatUsed;

            if (my_gps_data.lat < -0.00001 || my_gps_data.lat > 0.00001) {   // 当获取卫星定位信息不为零时才会校准
                if (eUnderCalib == regBuf[REG_CALIB_STATUS_REG]) {
                    setTime(my_gps_data.year, my_gps_data.month, my_gps_data.date, \
                        my_gps_data.hour, my_gps_data.minute, my_gps_data.second);
                    regBuf[REG_CALIB_STATUS_REG] = eCalibComplete;
                }
            }
            regBuf[REG_LAT_1] = (uint8_t)(my_gps_data.lat / 100.0);
            regBuf[REG_LAT_2] = (uint8_t)(((uint16_t)my_gps_data.lat % 100));
            xiaoshu = ((double)my_gps_data.lat - (uint16_t)my_gps_data.lat) * 100000;
            regBuf[REG_LAT_X_24] = (uint8_t)(xiaoshu >> 16);
            regBuf[REG_LAT_X_16] = (uint8_t)(xiaoshu >> 8);
            regBuf[REG_LAT_X_8] = (uint8_t)(xiaoshu);
            regBuf[REG_LAT_DIS] = (uint8_t)(my_gps_data.ew);                        // 这里把经纬度的方向给错了，应该给ns
            regBuf[REG_LON_1] = (uint8_t)(my_gps_data.lon / 100.0);
            regBuf[REG_LON_2] = (uint8_t)(((uint16_t)my_gps_data.lon % 100));
            xiaoshu = ((double)my_gps_data.lon - (uint16_t)my_gps_data.lon) * 100000;
            regBuf[REG_LON_X_24] = (uint8_t)(xiaoshu >> 16);
            regBuf[REG_LON_X_16] = (uint8_t)(xiaoshu >> 8);
            regBuf[REG_LON_X_8] = (uint8_t)(xiaoshu);
            regBuf[REG_LON_DIS] = (uint8_t)(my_gps_data.ns);                        // 这里把经纬度的方向给错了，应该给ew，使用库解决了这个问题，这里是错的

            regBuf[REG_ALT_H] = (uint8_t)((uint16_t)my_gps_data.alt >> 8);
            regBuf[REG_ALT_L] = (uint8_t)(my_gps_data.alt);
            regBuf[REG_ALT_X] = ((double)my_gps_data.alt - (uint16_t)my_gps_data.alt) * 100;

            //memcpy_gga();
        }
        // 解析VTG数据
        if (strncmp((const char*)(&p_data[list_number][3]), VTG, 4) == 0) {
            list_count = 0;
            memset(ss_data, 0, sizeof(ss_data));
            for (uint8_t i = 0; i < list_len; i++) {
                if (p_data[list_number][i] == ',') {
                    ss_data_len[list_count] = i - douhao_temp;
                    memcpy(&ss_data[list_count++][0], &p_data[list_number][douhao_temp], i - douhao_temp);
                    douhao_temp = i + 1;
                }
            }
            memset((void*)&gnvtg_data, 0, sizeof(gnvtg_data));
            memcpy(gnvtg_data.TalkerID, &ss_data[0][0], ss_data_len[0]);
            memcpy(gnvtg_data.cogt, &ss_data[1][0], ss_data_len[1]);
            memcpy(gnvtg_data.cogm, &ss_data[3][0], ss_data_len[3]);
            memcpy(gnvtg_data.sogn, &ss_data[5][0], ss_data_len[5]);
            memcpy(gnvtg_data.sogk, &ss_data[7][0], ss_data_len[7]);

            my_gps_data.cog = atof(gnvtg_data.cogt);
            my_gps_data.sog = atof(gnvtg_data.sogn);

            regBuf[REG_SOG_H] = (uint16_t)my_gps_data.sog >> 8;
            regBuf[REG_SOG_L] = (uint16_t)my_gps_data.sog;
            regBuf[REG_SOG_X] = ((double)my_gps_data.sog - (uint16_t)my_gps_data.sog) * 100;

            regBuf[REG_COG_H] = (uint16_t)my_gps_data.cog >> 8;
            regBuf[REG_COG_L] = (uint16_t)my_gps_data.cog;
            regBuf[REG_COG_X] = ((double)my_gps_data.cog - (uint16_t)my_gps_data.cog) * 100;
        }

        if (list_number < list_num - 1) {
            // 这里不计算最后一个gps数据包 这里的最后一个包的前缀和gps的前缀gp冲突
            // $GPTXT,01,01,01,ANTENNA OK*35
            if (strncmp((const char*)p_data[list_number], "$GP", 3) == 0) {
                talkID |= 0x01;
            }
            if (strncmp((const char*)p_data[list_number], "$BD", 3) == 0) {
                talkID |= 0x02;
            }
            if (strncmp((const char*)p_data[list_number], "$GL", 3) == 0) {
                talkID |= 0x04;
            }
        }
    }
    regBuf[REG_GNSS_MODE] = talkID & 0x07;
}
