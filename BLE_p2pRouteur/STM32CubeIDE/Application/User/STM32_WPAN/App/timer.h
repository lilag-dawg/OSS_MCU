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
#include "data_management.h"

#define TIM2_FREQUENCY 1000 //Hz
#define TIM2_PRESCALER_VALUE (uint32_t)(((SystemCoreClock) / (TIM2_FREQUENCY)) - 1)
#define TIM2_PERIOD 0xFFFFFFFF

#define TIM16_FREQUENCY 1000 //Hz
#define TIM16_PRESCALER_VALUE (uint32_t)(((SystemCoreClock) / (TIM16_FREQUENCY)) - 1)
#define TIM16_PERIOD 1500 //Algo Timer in ms

#define TIM17_FREQUENCY 1000 //Hz
#define TIM17_PRESCALER_VALUE (uint32_t)(((SystemCoreClock) / (TIM17_FREQUENCY)) - 1)
#define TIM17_PERIOD 50 //Relay settle time in ms


TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim16;
TIM_HandleTypeDef htim17;

void startSensorsTimer();
void startAlgoTimer();
float getSensorsTime();
void setupRelayTimer();
void startRelayTimer(uint16_t GPIO_PIN);

#endif /* APPLICATION_USER_STM32_WPAN_APP_TIMER_H_ */
