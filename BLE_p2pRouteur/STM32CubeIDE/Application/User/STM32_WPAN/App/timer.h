/*
 * timer.h
 *
 *  Created on: 22 nov. 2020
 *      Author: Carl
 */

#ifndef APPLICATION_USER_STM32_WPAN_APP_TIMER_H_
#define APPLICATION_USER_STM32_WPAN_APP_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define TIM2_FREQUENCY 1000 //Hz
#define TIM2_PRESCALER_VALUE (uint32_t)(((SystemCoreClock) / (TIM2_FREQUENCY)) - 1)
#define TIM2_PERIOD 0xFFFFFFFF

#define TIM16_FREQUENCY 1000 //Hz
#define TIM16_PRESCALER_VALUE (uint32_t)(((SystemCoreClock) / (TIM16_FREQUENCY)) - 1)
#define TIM16_PERIOD 500 //Algo Timer in ms



TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim16;

void startSensorsTimer();
void startAlgoTimer();
uint32_t getSensorsTime();

#endif /* APPLICATION_USER_STM32_WPAN_APP_TIMER_H_ */
