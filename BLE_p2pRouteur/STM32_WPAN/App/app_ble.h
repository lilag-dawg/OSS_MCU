/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_ble.h
  * @author  MCD Application Team
  * @brief   Header for ble application
  ******************************************************************************
  * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APP_BLE_H
#define APP_BLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hci_tl.h"
#include <stdbool.h>
#include "saveToFlash.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/

  typedef enum
    {
      APP_BLE_IDLE,
      APP_BLE_FAST_ADV,
      APP_BLE_LP_ADV,
      APP_BLE_SCAN,
      APP_BLE_LP_CONNECTING,
      APP_BLE_CONNECTED_SERVER,
      APP_BLE_CONNECTED_CLIENT,
	  APP_BLE_CONNECTED_CLIENT_NOTIFICATION_ENABLED,

      APP_BLE_DISCOVER_SERVICES,
      APP_BLE_DISCOVER_CHARACS,
      APP_BLE_DISCOVER_WRITE_DESC,
      APP_BLE_DISCOVER_NOTIFICATION_CHAR_DESC,
	  APP_BLE_READ_CHARACS,
      APP_BLE_ENABLE_NOTIFICATION_DESC,
      APP_BLE_DISABLE_NOTIFICATION_DESC,
	  APP_BLE_DISCOVER_READ_NOTIFICATION_CHAR_DESC
    } APP_BLE_ConnStatus_t;


typedef enum
{
  EDS_CONNEX_HAND_CARA_2,
  EDS_CONNEX_HAND_CARA_4,
} P2P_Router_Notification_evt_t;

typedef enum
{
  P2P_START_TIMER_EVT,
  P2P_STOP_TIMER_EVT,
  P2P_NOTIFICATION_INFO_RECEIVED_EVT,
  P2P_NOTIFICATION_CSC_RECEIVED_EVT,
  P2P_NOTIFICATION_SHIMANO_RECEIVED_EVT,
  P2P_NOTIFICATION_CP_RECEIVED_EVT,
} P2P_Client_Opcode_Notification_evt_t;

typedef struct
{
  uint8_t * pPayload;
  uint8_t     Length;
} P2P_Client_Data_t;

typedef struct
{
  P2P_Client_Opcode_Notification_evt_t  P2P_Client_Evt_Opcode;
  P2P_Client_Data_t DataTransfered;
  uint8_t   ServiceInstance;
} P2P_Client_App_Notification_evt_t;



/* USER CODE BEGIN ET */

#define MAX_DEVICES					63
#define MAX_DEVICE_NAME_LENGHT		19
#define SENSOR_NAME		 			"	Rideesense"
#define SENSOR_NAME2		 		"	CAD-BLE0967095"

typedef enum
{
  DISCONNECT,
  CONNECT,
  DISCONNECTING,
  CONNECTING,
} Pairing_request_status;



typedef struct
{
	bool cadence;
	bool speed;
	bool power;
	bool battery;
	bool gear;
} DeviceSupportedDataType;

typedef struct
{
  uint32_t time;
  float value;
} Algorithme_type_data;

struct BikeDataInformation_t{
	Algorithme_type_data cadence;
	Algorithme_type_data speed;
	Algorithme_type_data power;
	Algorithme_type_data pinion_fd;
	Algorithme_type_data pinion_rd;
};

typedef struct
{
    char deviceName[MAX_DEVICE_NAME_LENGHT];
    uint8_t macAddress[6];
    APP_BLE_ConnStatus_t pairingStatus;
    uint8_t position;
    DeviceSupportedDataType supportedDataType;
} DeviceInformations_t;

typedef struct
{
	uint8_t P2PServiceHandle;
	uint8_t P2PServiceEndHandle;

}ServiceHandle_t;

typedef struct
{
	// CSC
	ServiceHandle_t CSCServicehandle;

	// POWER
	ServiceHandle_t PowerServicehandle;

	//Shimano
	ServiceHandle_t ShimanoServicehandle;

	// notification
	uint16_t P2PNotificationCharHdle;
	uint16_t P2PNotificationDescHandle;
} ServicesHandleList_t;

typedef enum
{
	OTHER,
	CSC_SENSOR,
	POWER_SENSOR,
	TRAINER,
	SHIMANO_SENSOR,
} SensorType_t;

typedef struct
{
    char name[MAX_DEVICE_NAME_LENGHT];
    uint8_t macAddress[6];
    APP_BLE_ConnStatus_t state;
    uint16_t connHandle;
    bool isNotEmpty;
    ServicesHandleList_t servicesHandle;
    DeviceSupportedDataType supportedDataType;
    SensorType_t sensorType;
    P2P_Client_Opcode_Notification_evt_t sensor_evt_type; // un capteur peut notif une seule carac, A MODIFIER
} UsedDeviceInformations_t;


extern bool uuid_bit_format;
typedef struct
{
	DeviceInformations_t scannedDevicesList[MAX_DEVICES];
	uint8_t numberOfScannedDevices;
	uint8_t iterator;
} ScannedDevicesPackage_t;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions ---------------------------------------------*/
  void APP_BLE_Init( void );

  APP_BLE_ConnStatus_t APP_BLE_Get_Client_Connection_Status( uint16_t Connection_Handle );

  int APP_BLE_Get_Client_Connection_Index( uint16_t Connection_Handle );

  void Trigger_Scan_Request( void );
  void Trigger_Connection_Request( int index, int indexInScannedDevices, Pairing_request_status status );

/* USER CODE BEGIN EF */
  int getCorrespondingIndex(char* sensorName);
/* USER CODE END EF */

#ifdef __cplusplus
}
#endif

#endif /*APP_BLE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
