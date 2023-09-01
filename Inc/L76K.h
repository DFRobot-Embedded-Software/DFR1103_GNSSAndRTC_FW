#ifndef __L76K_H__
#define __L76K_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"


#define GGA "GGA,"
#define VTG "VTG,"
#define ZDA "ZDA,"

extern uint16_t gps_offset;

typedef struct ggaData {
    char TalkerID[10];
    char utc[15];
    char lat[15];
    char ns[2];
    char lon[15];
    char ew[2];
    char Quality[2];
    char NumSatUsed[3];
    char HDOP[15];
    char alt[10];
    char sep[10];
    char DiffAge[10];
    char DiffStation[10];
}s_gga;

typedef struct vtgData {
    char TalkerID[10];
    char cogt[15];
    char cogm[15];
    char sogn[15];
    char sogk[15];
    char modeind[3];
}s_vtg;

typedef struct zdaData {
    char TalkerID[10];
    char utc[10];
    char day[5];
    char month[5];
    char year[7];
    char ltzh[5];
    char ltzn[5];
}s_zda;

typedef struct gpsData {
    uint16_t year;
    uint8_t month;
    uint8_t date;
    uint8_t hour;
     uint8_t minute;
    uint8_t second;
    double lat;
    uint8_t ew;
    double lon;
    uint8_t ns;
    uint8_t numSatUsed;     //使用的卫星数
    double alt;            //大地高度
    double sog;
    double cog;
}s_gps;

typedef struct utc {
    uint8_t hour[5];
    uint8_t minute[5];
    uint8_t second[5];
}s_utc;


void enable_gnss_power(void);
void disable_gnss_power(void);

uint8_t hex_to_ascll_str(uint8_t data);

void init_l76k_gpio(void);

void set_uart_baud(char* baud);
void set_star_type(char * type);
void set_reset_mode(char * mode);
void set_frequency(char * frequency);

void anaysis_gps_data(uint8_t* data, uint16_t datalen);


#ifdef __cplusplus
}
#endif

#endif /* __L76K_H__ */
