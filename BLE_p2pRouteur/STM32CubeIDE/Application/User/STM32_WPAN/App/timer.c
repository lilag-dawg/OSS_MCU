/*
 * timer.c
 *
 *  Created on: 22 nov. 2020
 *      Author: Carl
 */

/* Includes ------------------------------------------------------------------*/
#include "timer.h"
/* Private typedef -----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/


void startSensorsTimer()
{

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = TIM2_PRESCALER_VALUE;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = TIM2_PERIOD;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
	{
	Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
	{
	Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
	{
	Error_Handler();
	}

	__HAL_TIM_ENABLE(&htim2);
}

void startAlgoTimer() {
	htim16.Instance = TIM16;
	htim16.Init.Prescaler = TIM16_PRESCALER_VALUE;
	htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim16.Init.Period = TIM16_PERIOD;
	htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim16.Init.RepetitionCounter = 0;
	htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
	{
		Error_Handler();
	}

	if (HAL_TIM_Base_Start_IT(&htim16) != HAL_OK)
	{
		Error_Handler();
	}
}

uint32_t getSensorsTime()
{
	return __HAL_TIM_GET_COUNTER(&htim2);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  BSP_LED_Toggle(LED2);
  //Call de l'algo ici
  algoCases();
}
