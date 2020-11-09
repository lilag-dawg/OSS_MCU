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

      APP_BLE_DISCOVER_SERVICES,
      APP_BLE_DISCOVER_CHARACS,
      APP_BLE_DISCOVER_WRITE_DESC,
      APP_BLE_DISCOVER_NOTIFICATION_CHAR_DESC,
      APP_BLE_ENABLE_NOTIFICATION_DESC,
      APP_BLE_DISABLE_NOTIFICATION_DESC
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
} P2P_Client_Opcode_Notification_evt_t;

typedef struct
{
  uint8_t * pPayload;
  uint8_t     Length;
}P2P_Client_Data_t;

typedef struct
{
  P2P_Client_Opcode_Notification_evt_t  P2P_Client_Evt_Opcode;
  P2P_Client_Data_t DataTransfered;
  uint8_t   ServiceInstance;
}P2P_Client_App_Notification_evt_t;



/* USER CODE BEGIN ET */

#define MAX_DEVICES					127
#define MAX_DEVICE_NAME_LENGHT		19

typedef struct{
	bool cadence;
	bool speed;
	bool power;
	bool battery;
	bool gear;
}DeviceSupportedDataType;

struct DeviceInformations_t{
    char deviceName[MAX_DEVICE_NAME_LENGHT];
    uint8_t deviceAddress[6];
    uint8_t pairingStatus;
    uint8_t position;
    DeviceSupportedDataType supportedDataType;
};

extern struct DeviceInformations_t devicesList[MAX_DEVICES];
extern int device_list_index;

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

/* USER CODE BEGIN EF */
  void APP_BLE_Key_Button1_Action(void);
  void APP_BLE_Key_Button2_Action(void);
  void APP_BLE_Key_Button3_Action(void);
/* USER CODE END EF */

#ifdef __cplusplus
}
#endif

#endif /*APP_BLE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
