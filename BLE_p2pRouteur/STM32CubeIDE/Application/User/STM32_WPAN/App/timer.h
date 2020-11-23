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

#define TIMER_FREQUENCY 1000 //Hz
#define PRESCALER_VALUE (uint32_t)(((SystemCoreClock) / (TIMER_FREQUENCY)) - 1)

TIM_HandleTypeDef htim2;

void startSensorsTimer();
uint32_t getSensorsTime();

#endif /* APPLICATION_USER_STM32_WPAN_APP_TIMER_H_ */
