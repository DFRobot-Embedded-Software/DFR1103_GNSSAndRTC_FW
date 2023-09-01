#include "timer.h"

volatile uint8_t cs32TimerFlag = 0;
volatile uint8_t l76kTimerFlag = 0;
volatile uint16_t secTimerFlag = 0;
volatile uint16_t minTimerFlag = 0;

BASETIM_HandleTypeDef sBaseTimHandle = {0};

/**
  * @brief  Period elapsed callback in non blocking mode 
  * @param  htim : BASETIM handle
  * @retval None
  */
void HAL_BASETIM_PeriodElapsedCallback(BASETIM_HandleTypeDef *htim)
{
  cs32TimerFlag++;
  l76kTimerFlag++;
  secTimerFlag++;
  if (secTimerFlag > 60000) {
    secTimerFlag = 0;
    minTimerFlag++;
  }
}

void uartAnalysisTimerInit(void)
{
  __HAL_RCC_BASETIM_CLK_ENABLE();

  // 系统时钟为24M,且该定时器只能向上计数
  // 24 000 / 24 000 000 = 1 / 1000 s = 1 ms
  sBaseTimHandle.Instance = TIM11;                                 // TIM11
  sBaseTimHandle.Init.CntTimSel = BASETIM_TIMER_SELECT;            // 选择为定时器功能
  sBaseTimHandle.Init.AutoReload = BASETIM_AUTORELOAD_ENABLE;      // 使能自动重装载
  sBaseTimHandle.Init.MaxCntLevel = BASETIM_MAXCNTLEVEL_16BIT;     // 设置计数器的最大计数值为16bit
  sBaseTimHandle.Init.OneShot = BASETIM_REPEAT_MODE;               // 计数器运行模式为重复模式
  sBaseTimHandle.Init.Prescaler = BASETIM_PRESCALER_DIV64;         // 预分频系数为64, 24000 计数值为 64ms
  sBaseTimHandle.Init.Period = BASETIM_MAXCNTVALUE_16BIT - 24000 / 64;  //设置计数器周期装载值, 延时1ms (多一个计数值的影响)

  if (HAL_BASETIM_Base_Init(&sBaseTimHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  HAL_NVIC_EnableIRQ(TIM11_IRQn);

  HAL_NVIC_SetPriority(TIM11_IRQn, 0);

  if (HAL_BASETIM_Base_Start_IT(&sBaseTimHandle) != HAL_OK)
  {
    /* Starting Error */
    Error_Handler();
  }
  cs32TimerFlag = 0;
  l76kTimerFlag = 0;
}
