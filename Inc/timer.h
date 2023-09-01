/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIMER_H
#define __TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "main.h"

/* Includes ------------------------------------------------------------------*/

extern volatile uint8_t cs32TimerFlag, l76kTimerFlag;
extern volatile uint16_t secTimerFlag, minTimerFlag;
extern BASETIM_HandleTypeDef sBaseTimHandle;

/* Exported functions prototypes ---------------------------------------------*/

/*
 * Initalize system-timeout counter timer-4
 */
void uartAnalysisTimerInit(void);

/* Private defines -----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __TIMER_H */
