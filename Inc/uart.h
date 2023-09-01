#ifndef __UART_H__
#define __UART_H__
#include "main.h"

#define UART_BAUDRATE        115200

#define UART0_MAX_LEN        128
#define UART1_MAX_LEN        (1024 + 300)

#define UART0_READ_REGBUF    0xBB
#define UART0_WRITE_REGBUF   0xCC

extern uint8_t cs32Uart0Buffer[UART0_MAX_LEN], l76kUart1Buffer[UART1_MAX_LEN];

extern volatile uint8_t cs32RxCount, cs32HandleCount;
extern volatile uint16_t l76kRxCount;

/**
 * @brief 初始化串口
 *
 * @param baud 波特率
 */
void initUart0ForCS32(uint32_t baud);
void initUart1ForL76K(uint32_t baud);

/**
 * @brief 发送串口数据
 *
 * @param data 指向待发送数据的指针
 * @param len 待发送数据长度
 */
void uart0Send(uint8_t* data, uint8_t len);
void uart1Send(uint8_t* data, uint8_t len);

void annysis_uart0_command(void);

#endif
