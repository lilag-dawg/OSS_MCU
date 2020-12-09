/*
 * data_management.h
 *
 *  Created on: 8 nov. 2020
 *      Author: Isam
 */

#ifndef APPLICATION_USER_STM32_WPAN_APP_DATA_MANAGEMENT_H_
#define APPLICATION_USER_STM32_WPAN_APP_DATA_MANAGEMENT_H_

#include "app_ble.h"

void switchCase(int* value, SensorType_t sensorType);
void wheelRevFunction(int* wheelValue);
void crankRevFunction(int* CrankValue);
void powerFunction(int* powerData);
void GetRatio(int *tableau);
void algoCases(void);

#endif /* APPLICATION_USER_STM32_WPAN_APP_DATA_MANAGEMENT_H_ */
