/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    p2p_routeur_app.c
 * @author  MCD Application Team
 * @brief   P2P Routeur Application
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

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "app_common.h"
#include "dbg_trace.h"
#include "ble.h"
#include "app_ble.h"
#include "p2p_routeur_app.h"
#include "stm32_seq.h"
#include "data_management.h"
#include "timer.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

typedef struct
{
  /**
   * state of the P2P Client
   * state machine
   */
  APP_BLE_ConnStatus_t state;

  /**
   * connection handle
   */
  uint16_t connHandle;

  /**
   * handle of the P2P service
   */
  uint16_t P2PServiceHandle;

  /**
   * end handle of the P2P service
   */
  uint16_t P2PServiceEndHandle;

  /**
   * handle of the Tx characteristic - Write To Server
   *
   */
  uint16_t P2PWriteToServerCharHdle;

  /**
   * handle of the client configuration
   * descriptor of Tx characteristic
   */
  uint16_t P2PWriteToServerDescHandle;

  /**
   * handle of the Rx characteristic - Notification From Server
   *
   */
  uint16_t P2PNotificationCharHdle;

  /**
   * handle of the client configuration
   * descriptor of Rx characteristic
   */
  uint16_t P2PNotificationDescHandle;

  /**
   * handle of the Rx characteristic - Notification From Server
   *
   */
  uint16_t P2PReadCharHdle;

  uint16_t P2PcurrentCharBeingRead;

  P2P_Client_Opcode_Notification_evt_t sensor_evt_type;

}P2P_ClientContext_t;

/* USER CODE BEGIN PTD */


typedef struct{
	uint8_t Value;
	uint8_t CurrentPosition;
}Gestion_Conn_NbrSensor_t;


typedef struct{
	Pairing_request_status status;
	char SensorName[19];
}Gestion_Conn_PairingRequest_t;

typedef struct{
	char SensorName[19];
	uint8_t dataType;
}Gestion_Conn_Datatype_t;

typedef struct
{
	P2P_Router_Notification_evt_t state;

	Gestion_Conn_NbrSensor_t 		NumberOfSensorNearbyStruct;
	Gestion_Conn_PairingRequest_t	PairingRequestStruct;
	Gestion_Conn_Datatype_t			SensorDataTypeStruct;

    uint8_t Update_timer_Id_CONN_HAND_CARA_2;
    uint8_t Update_timer_Id_CONN_HAND_CARA_4;
   

} P2P_Router_App_Context_t;



/* Private defines ------------------------------------------------------------*/
#define UNPACK_2_BYTE_PARAMETER(ptr)  \
        (uint16_t)((uint16_t)(*((uint8_t *)ptr))) |   \
        (uint16_t)((((uint16_t)(*((uint8_t *)ptr + 1))) << 8))

#define MAX_NAMES							8
#define MAX_CARAC							19


#define NAME_CHANGES_PERIODE			(0.1*1000*1000/CFG_TS_TICK_VAL) //100ms//

int sensorIndexForServices = 0;
bool isFirstTimerInit = true;
/* USER CODE BEGIN PD */

extern ScannedDevicesPackage_t scannedDevicesPackage;
extern UsedDeviceInformations_t usedDeviceInformations[4];
Data_per_service_t Data_per_service;


/* USER CODE END PD */

/* Private macros -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/**
 * START of Section BLE_APP_CONTEXT
 */

PLACE_IN_SECTION("BLE_APP_CONTEXT") static P2P_ClientContext_t aP2PClientContext[BLE_CFG_CLT_MAX_NBR_CB];

/**
 * END of Section BLE_APP_CONTEXT
 */
/* USER CODE BEGIN PV */
PLACE_IN_SECTION("BLE_APP_CONTEXT") static P2P_Router_App_Context_t P2P_Router_App_Context;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

static SVCCTL_EvtAckStatus_t Client_Event_Handler(void *pckt);
static void Server_Update_Service( void );
static void Client_Update_Service( void );
void P2P_Router_APP_Init(void);
void P2P_Client_App_Notification(P2P_Client_App_Notification_evt_t *pNotification);
void P2P_Client_Init(void);

int getCorrespondingIndex(uint8_t* macAddress);
static void Update_Paired_Devices_In_Flash(void);
/* USER CODE BEGIN PFP */


static void P2P_SensorDataType_Timer_Callback(void){
	UTIL_SEQ_SetTask(1<<CFG_TASK_SEND_DATA_TYPE_ID, CFG_SCH_PRIO_1 );
}

static void P2P_SensorName_Timer_Callback(void){
	UTIL_SEQ_SetTask(1<<CFG_TASK_SEND_SENSOR_NAMES_ID, CFG_SCH_PRIO_1 );
}


uint8_t* getCorrespondingMacAdress(char* sensorName){

	//uint8_t macNotFound[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

	for(int i = 0; i<scannedDevicesPackage.numberOfScannedDevices;i++){
		if(strcmp(sensorName, scannedDevicesPackage.scannedDevicesList[i].deviceName) == 0){
			return scannedDevicesPackage.scannedDevicesList[i].macAddress;
		}
	}
	return NULL;
}


static void Update_Paired_Devices_In_Flash(void){
    struct settings readSettings;
    struct settings settingsToWrite;

    int indexOfDeviceList = 0;
    uint8_t* currentMacAdd = getCorrespondingMacAdress(P2P_Router_App_Context.PairingRequestStruct.SensorName);
    indexOfDeviceList = getCorrespondingIndex(currentMacAdd);

    int sum = 0;
    int index = 0;
    uint16_t connhandle = 0;

    readFlash((uint8_t*)&readSettings);
    settingsToWrite = readSettings;

    if(P2P_Router_App_Context.PairingRequestStruct.status == CONNECTING){
    	for(int i = 0; i < sizeof(settingsToWrite.sensors); i++){
    		sum = 0;
    		for(int k = 0; k < sizeof(settingsToWrite.sensors[i].name);k++){
    			sum |= settingsToWrite.sensors[i].name[k];
    		}
    		if(sum == 0){ //space is empty
    			strcpy(settingsToWrite.sensors[i].name, P2P_Router_App_Context.PairingRequestStruct.SensorName);
    			memcpy(settingsToWrite.sensors[i].macAddress, scannedDevicesPackage.scannedDevicesList[indexOfDeviceList].macAddress, sizeof(settingsToWrite.sensors[i].macAddress));
    			index = i;
    			break;
    		}
    	}
    }
    else{ // request to disconnect
        for(int i = 0; i < sizeof(readSettings.sensors); i++){
        	if(strcmp(P2P_Router_App_Context.PairingRequestStruct.SensorName, settingsToWrite.sensors[i].name) == 0){
        		memset(settingsToWrite.sensors[i].name, 0, sizeof(settingsToWrite.sensors[i].name));
        		memset(settingsToWrite.sensors[i].macAddress, 0, sizeof(settingsToWrite.sensors[i].macAddress));
        		index = i;
        	}
        }
    }
    saveToFlash((uint8_t*) &settingsToWrite, sizeof(settingsToWrite));

    connhandle = Update_UsedDeviceInformations_structure();

    Trigger_Connection_Request(index,indexOfDeviceList,P2P_Router_App_Context.PairingRequestStruct.status, connhandle);
}


/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
/**
 * @brief  End Device service Notification
 * @param  GATT Notification (Opcode & Data)
 * @retval None
 */
void EDS_STM_App_Notification(EDS_STM_App_Notification_evt_t *pNotification)
{
/* USER CODE BEGIN EDS_STM_App_Notification_1 */

/* USER CODE END EDS_STM_App_Notification_1 */
        switch(pNotification->EDS_Evt_Opcode)
    {
        /* USER CODE BEGIN EDS_Evt_Opcode */

        /* USER CODE END EDS_Evt_Opcode */
        case EDS_CONNEX_HAND_CARA_2_NOTIFY_ENABLED_EVT:
#if(CFG_DEBUG_APP_TRACE != 0)
            APP_DBG_MSG("-- APPLICATION ROUTER : ENDDEVICEMGT NOTIFICATION ENABLED\r\n");
#endif
            /* USER CODE BEGIN EDS_STM_NOTIFY_ENABLED_EVT */
            P2P_Router_App_Context.state = EDS_CONNEX_HAND_CARA_2;
            HW_TS_Start(P2P_Router_App_Context.Update_timer_Id_CONN_HAND_CARA_2, NAME_CHANGES_PERIODE);
            /* USER CODE END EDS_STM_NOTIFY_ENABLED_EVT */
            break;

        case EDS_CONNEX_HAND_CARA_2_NOTIFY_DISABLED_EVT:
#if(CFG_DEBUG_APP_TRACE != 0)
            APP_DBG_MSG("-- APPLICATION ROUTER : ENDDEVICEMGT NOTIFICATION DISABLED\r\n");
#endif
            /* USER CODE BEGIN EDS_STM_NOTIFY_DISABLED_EVT */
            HW_TS_Stop(P2P_Router_App_Context.Update_timer_Id_CONN_HAND_CARA_2);
            /* USER CODE END EDS_STM_NOTIFY_DISABLED_EVT */
            break;
        case EDS_CONNEX_HAND_CARA_4_NOTIFY_ENABLED_EVT:
#if(CFG_DEBUG_APP_TRACE != 0)
            APP_DBG_MSG("-- APPLICATION ROUTER : ENDDEVICEMGT NOTIFICATION ENABLED\r\n");
#endif
            /* USER CODE BEGIN EDS_STM_NOTIFY_ENABLED_EVT */
            P2P_Router_App_Context.state = EDS_CONNEX_HAND_CARA_4;
            HW_TS_Start(P2P_Router_App_Context.Update_timer_Id_CONN_HAND_CARA_4, NAME_CHANGES_PERIODE);
            /* USER CODE END EDS_STM_NOTIFY_ENABLED_EVT */
            break;

        case EDS_CONNEX_HAND_CARA_4_NOTIFY_DISABLED_EVT:
#if(CFG_DEBUG_APP_TRACE != 0)
            APP_DBG_MSG("-- APPLICATION ROUTER : ENDDEVICEMGT NOTIFICATION DISABLED\r\n");
#endif
            /* USER CODE BEGIN EDS_STM_NOTIFY_DISABLED_EVT */
            HW_TS_Stop(P2P_Router_App_Context.Update_timer_Id_CONN_HAND_CARA_4);
            /* USER CODE END EDS_STM_NOTIFY_DISABLED_EVT */
            break;
        case EDS_CONNEX_HAND_CARA_3_WRITE_EVT:
#if(CFG_DEBUG_APP_TRACE != 0)
        	APP_DBG_MSG("-- GATT : WRITE CHAR INFO RECEIVED\n");
#endif
            /* USER CODE BEGIN EDS_CONNEX_HAND_CARA_3_WRITE_EVT */

        	//P2P_Router_App_Context.PairingRequestStruct.Pairing = pNotification->DataTransfered.pPayload[0];

        	switch(pNotification->DataTransfered.pPayload[0]){
				case CONNECT:
					P2P_Router_App_Context.PairingRequestStruct.status = CONNECTING;
					break;
				case DISCONNECT:
					//todo, handle disconnection
					P2P_Router_App_Context.PairingRequestStruct.status = DISCONNECTING;
					break;
				default:
					break;
        	}
			memset(&P2P_Router_App_Context.PairingRequestStruct.SensorName, 0, sizeof(P2P_Router_App_Context.PairingRequestStruct.SensorName));

			for(int i=1; i<pNotification->DataTransfered.Length;i++){
            	P2P_Router_App_Context.PairingRequestStruct.SensorName[i-1] = pNotification->DataTransfered.pPayload[i];
            }

			UTIL_SEQ_SetTask(  1<<CFG_TASK_UPDATE_FLASH, CFG_SCH_PRIO_0 );


            /* USER CODE END EDS_CONNEX_HAND_CARA_3_WRITE_EVT */
            break;
        case EDS_CALIBRATION_CARA_1_WRITE_EVT:
#if(CFG_DEBUG_APP_TRACE != 0)
        	APP_DBG_MSG("-- GATT : WRITE CHAR INFO RECEIVED\n");
#endif
            /* USER CODE BEGIN EDS_CALIBRATION_CARA_1_WRITE_EVT */

            /* USER CODE END EDS_CALIBRATION_CARA_1_WRITE_EVT */
            break;

        default:
            /* USER CODE BEGIN EDS_Evt_Opcode_default */

            /* USER CODE END EDS_Evt_Opcode_default */
            break;
    }
/* USER CODE BEGIN EDS_STM_App_Notification_2 */

/* USER CODE END EDS_STM_App_Notification_2 */
    return;
}
/**
 * @brief  P2P service Notification
 * @param  GATT Notification (Opcode & Data)
 * @retval None
 */
void P2PS_STM_App_Notification(P2PS_STM_App_Notification_evt_t *pNotification)
{
/* USER CODE BEGIN P2PS_STM_App_Notification_1 */

/* USER CODE END P2PS_STM_App_Notification_1 */
    switch(pNotification->P2P_Evt_Opcode)
    {
        /* USER CODE BEGIN P2P_Evt_Opcode */

        /* USER CODE END P2P_Evt_Opcode */
        case P2PS_STM__NOTIFY_ENABLED_EVT:
#if(CFG_DEBUG_APP_TRACE != 0)
            APP_DBG_MSG("-- APPLICATION ROUTER : BUTTON NOTIFICATION ENABLED\n\r");
#endif
            /* USER CODE BEGIN P2PS_STM__NOTIFY_ENABLED_EVT */

            /* USER CODE END P2PS_STM__NOTIFY_ENABLED_EVT */
            break;

        case P2PS_STM_NOTIFY_DISABLED_EVT:
#if(CFG_DEBUG_APP_TRACE != 0)
            APP_DBG_MSG("-- APPLICATION ROUTER : BUTTON NOTIFICATION DISABLED\n\r");
#endif
            /* USER CODE BEGIN P2PS_STM_NOTIFY_DISABLED_EVT */
            /* USER CODE END P2PS_STM_NOTIFY_DISABLED_EVT */
            break;

        case P2PS_STM_READ_EVT:
#if(CFG_DEBUG_APP_TRACE != 0)
            APP_DBG_MSG("-- APPLICATION ROUTER : LED READ STATUS\n\r");
#endif
            /* USER CODE BEGIN P2PS_STM_READ_EVT */

            /* USER CODE END P2PS_STM_READ_EVT */
            break;

        case P2PS_STM_WRITE_EVT:
#if(CFG_DEBUG_APP_TRACE != 0)
            APP_DBG_MSG("-- APPLICATION ROUTER : WRITE LED CONFIG TO ALL END DEVICE \n\r");
#endif
            /* USER CODE BEGIN P2PS_STM_WRITE_EVT */

            /* USER CODE END P2PS_STM_WRITE_EVT */
            break;

        default:
            /* USER CODE BEGIN P2P_Evt_Opcode_default */

            /* USER CODE END P2P_Evt_Opcode_default */
        break;
    }
/* USER CODE BEGIN P2PS_STM_App_Notification_2 */

/* USER CODE END P2PS_STM_App_Notification_2 */
    return;
}

/**
 * @brief  Router Application initialisation
 * @param  None
 * @retval None
 */
void P2P_Router_APP_Init(void)
{
    /* USER CODE BEGIN P2P_Router_APP_Init_1 */

    /* USER CODE END P2P_Router_APP_Init_1 */

    //UTIL_SEQ_RegTask(1<<CFG_TASK_SEARCH_SERVICE_ID, UTIL_SEQ_RFU, Client_Update_Service );

	//test for seq
	UTIL_SEQ_RegTask(1<<CFG_TASK_UPDATE_FLASH, UTIL_SEQ_RFU, Update_Paired_Devices_In_Flash );

	// for CONNEX_HAND_CARA_2
    UTIL_SEQ_RegTask(1<<CFG_TASK_SEND_SENSOR_NAMES_ID, UTIL_SEQ_RFU, Server_Update_Service );
    HW_TS_Create(CFG_TIM_PROC_ID_ISR, &(P2P_Router_App_Context.Update_timer_Id_CONN_HAND_CARA_2), hw_ts_Repeated, P2P_SensorName_Timer_Callback);

    // for CONNEX_HAND_CARA_4
    UTIL_SEQ_RegTask(1<<CFG_TASK_SEND_DATA_TYPE_ID, UTIL_SEQ_RFU, Server_Update_Service );
    HW_TS_Create(CFG_TIM_PROC_ID_ISR, &(P2P_Router_App_Context.Update_timer_Id_CONN_HAND_CARA_4), hw_ts_Repeated, P2P_SensorDataType_Timer_Callback);

    Data_per_service.isPowerReceived = false;
    Data_per_service.isCSCReceived = false;
    Data_per_service.isShimanoReceived = false;
    /* USER CODE BEGIN P2P_Router_APP_Init_2 */
    /**
     * Initialize LedButton Service
     */

    P2P_Router_App_Context.NumberOfSensorNearbyStruct.CurrentPosition = 0;
    P2P_Router_App_Context.PairingRequestStruct.status = DISCONNECT;
    memset(&P2P_Router_App_Context.PairingRequestStruct.SensorName, 0, sizeof(P2P_Router_App_Context.PairingRequestStruct.SensorName));

#if (CFG_P2P_DEMO_MULTI != 0 )   

#endif
    /* Update EndDevice Management Service */
    
    

    /* USER CODE END P2P_Router_APP_Init_2 */

    return;
}





/**
 * @brief  Notification from the Server
 * @param  Client Notification from Server side
 * @retval None
 */
void P2P_Client_App_Notification(P2P_Client_App_Notification_evt_t *pNotification)
{
/* USER CODE BEGIN P2P_Client_App_Notification_1 */
	int sensorData[11] = {0};
	int tab[17] = {0};

/* USER CODE END P2P_Client_App_Notification_1 */

    switch(pNotification->P2P_Client_Evt_Opcode)
    {
    /* USER CODE BEGIN P2P_Client_Evt_Opcode */
    case P2P_NOTIFICATION_SHIMANO_RECEIVED_EVT:

    	for(int i = 0; i<pNotification->DataTransfered.Length; i++){
			tab[i] = pNotification->DataTransfered.pPayload[i];
		}
		GetRatio(tab);
		Data_per_service.isShimanoReceived = true;
	break;

    case P2P_NOTIFICATION_CSC_RECEIVED_EVT:

    	for(int i = 0; i<pNotification->DataTransfered.Length; i++){
			sensorData[i] = pNotification->DataTransfered.pPayload[i];
		}

		switchCase(sensorData);
		Data_per_service.isCSCReceived = true;
	break;

    case P2P_NOTIFICATION_CP_RECEIVED_EVT:
    	for(int i = 0; i<2; i++){
			sensorData[i] = pNotification->DataTransfered.pPayload[i+1];
		}

    	powerFunction(sensorData);
    	Data_per_service.isPowerReceived = true;

    break;
    /* USER CODE END P2P_Client_Evt_Opcode */
        default:
    /* USER CODE BEGIN P2P_Client_Evt_Opcode_default */

    /* USER CODE END P2P_Client_Evt_Opcode_default */
        break;

    }
/* USER CODE BEGIN P2P_Client_App_Notification_2 */
    if(Data_per_service.isShimanoReceived && Data_per_service.isCSCReceived && Data_per_service.isPowerReceived && isFirstTimerInit){
        //startAlgoTimer();
        isFirstTimerInit = false;
    }

/* USER CODE END P2P_Client_App_Notification_2 */
    return;
}

/**
 * @brief  Service initialization
 * @param  None
 * @retval None
 */
void P2P_Client_Init(void)
{
    uint8_t index =0;
/* USER CODE BEGIN P2P_Client_Init_1 */
    UTIL_SEQ_RegTask( 1<< CFG_TASK_SEARCH_SERVICE_ID, UTIL_SEQ_RFU, Client_Update_Service );

/* USER CODE END P2P_Client_Init_1 */

    for(index = 0; index < BLE_CFG_CLT_MAX_NBR_CB; index++)
        aP2PClientContext[index].state= APP_BLE_IDLE;
    /**
     *    Register the event handler to the BLE controller
     */
    SVCCTL_RegisterCltHandler(Client_Event_Handler);
/* USER CODE BEGIN P2P_Client_Init_2 */
    APP_DBG_MSG("-- LED BUTTON CLIENT INITIALIZED \n");
/* USER CODE END P2P_Client_Init_2 */
    return;
}

/* USER CODE BEGIN FD */
int getCorrespondingIndex(uint8_t* macAddress){
	for(int i = 0; i<scannedDevicesPackage.numberOfScannedDevices;i++){
		if(memcmp(macAddress, scannedDevicesPackage.scannedDevicesList[i].macAddress, sizeof(scannedDevicesPackage.scannedDevicesList[i].macAddress)) == 0){
			return i;
		}
	}
	return -1;
}
/* USER CODE END FD */
/*************************************************************
 *
 * PUBLIC FUNCTIONS
 *
 *************************************************************/
void P2PR_APP_Init( void )
{
    P2P_Router_APP_Init();
    P2P_Client_Init();
    /* USER CODE BEGIN P2PR_APP_Init */

    /* USER CODE END P2PR_APP_Init */
    return;
}

/* USER CODE BEGIN FD_PUBLIC_FUNCTIONS */

/* USER CODE END FD_PUBLIC_FUNCTIONS */
/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
static void Server_Update_Service( void )
{
	switch(P2P_Router_App_Context.state){
		case EDS_CONNEX_HAND_CARA_2:
		{
		    /* USER CODE BEGIN Server_Update_Service */

			uint8_t value[20];
			uint8_t index =  P2P_Router_App_Context.NumberOfSensorNearbyStruct.CurrentPosition;

			Pairing_request_status status = DISCONNECT;

			switch(scannedDevicesPackage.scannedDevicesList[index].pairingStatus){
				case APP_BLE_LP_CONNECTING:
					status = CONNECTING;
					break;
				case APP_BLE_CONNECTED_CLIENT:
					status = CONNECT;
					break;
				case APP_BLE_IDLE:
					status = DISCONNECT;
					break;
				default:
					break;
			}

			value[0] = (uint8_t)((scannedDevicesPackage.scannedDevicesList[index].position) << 2) + status; // PPPP PPCC

		    //green led is on when notifying
		    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);


			//printf("Size: %d \n\r",sizeof(scannedDevicesPackage.scannedDevicesList[index].deviceName));
			//printf("Position: %d \n\r",scannedDevicesPackage.scannedDevicesList[index].position);
			//printf("[");

			//printf("%x,",value[0]);

			for(int i = 1; i<(sizeof(value));i++){
				value[i] = (uint8_t)(scannedDevicesPackage.scannedDevicesList[index].deviceName[i-1]);
				//printf("%x,",value[i]);
			}

		    printf("status : %d name: %s\n\r", status, scannedDevicesPackage.scannedDevicesList[index].deviceName);


			P2P_Router_App_Context.NumberOfSensorNearbyStruct.CurrentPosition ++;

			if (P2P_Router_App_Context.NumberOfSensorNearbyStruct.CurrentPosition >= scannedDevicesPackage.numberOfScannedDevices){
				P2P_Router_App_Context.NumberOfSensorNearbyStruct.CurrentPosition = 0;
				printf("END OF PACKET\n\r");
			}

			//printf("]\n\r");

			EDS_STM_Update_Char(0x0000,(uint8_t *)&value);
		}

			break;
		case EDS_CONNEX_HAND_CARA_4:
		{
			uint8_t value[20];
			uint8_t index =  P2P_Router_App_Context.NumberOfSensorNearbyStruct.CurrentPosition;

			uint8_t isCadenceSupported = (uint8_t)((scannedDevicesPackage.scannedDevicesList[index].supportedDataType.cadence) << 4);
			uint8_t isSpeedSupported = (uint8_t)((scannedDevicesPackage.scannedDevicesList[index].supportedDataType.speed) << 3);
			uint8_t isPowerSupported = (uint8_t)((scannedDevicesPackage.scannedDevicesList[index].supportedDataType.power) << 2);
			uint8_t isBatteryupported = (uint8_t)((scannedDevicesPackage.scannedDevicesList[index].supportedDataType.battery) << 1);
			uint8_t isGearSupported = (uint8_t)((scannedDevicesPackage.scannedDevicesList[index].supportedDataType.gear));


			value[0] = (uint8_t)(isCadenceSupported + isSpeedSupported + isPowerSupported + isBatteryupported + isGearSupported);

			for(int i = 1; i<(sizeof(value));i++){
				value[i] = (uint8_t)(scannedDevicesPackage.scannedDevicesList[index].deviceName[i-1]);
			}

			P2P_Router_App_Context.NumberOfSensorNearbyStruct.CurrentPosition ++;

			if (P2P_Router_App_Context.NumberOfSensorNearbyStruct.CurrentPosition >= scannedDevicesPackage.numberOfScannedDevices){
				P2P_Router_App_Context.NumberOfSensorNearbyStruct.CurrentPosition = 0;
			}

			printf("[");

			for(int i = 0; i<(sizeof(value));i++){
				printf("%d,",value[i]);
			}

			printf("]\n\r");

			EDS_STM_Update_Char(0x0003,(uint8_t *)&value);

		}
			break;
	    default:

	        break;
	}

    return;
}

/**
 * @brief  Feature Characteristic update
 * @param  pFeatureValue: The address of the new value to be written
 * @retval None
 */

static void Client_Update_Service( void )
{
	  uint16_t enable = 0x0001;
	  uint16_t disable = 0x0000;

	  uint8_t index;

	  index = 0;
	  while((index < BLE_CFG_CLT_MAX_NBR_CB) &&
	          (usedDeviceInformations[index].state != APP_BLE_IDLE))
	  {


	    switch(usedDeviceInformations[index].state)
	    {

	      case APP_BLE_DISCOVER_SERVICES:
	        APP_DBG_MSG("P2P_DISCOVER_SERVICES\n");
	        break;
	      case APP_BLE_DISCOVER_CHARACS:
	    	  switch(usedDeviceInformations[index].sensorType){
	    	  case CSC_SENSOR:
	    		  APP_DBG_MSG("* GATT : Discover P2P Characteristics CSC sensor\n");
				  aci_gatt_disc_all_char_of_service(usedDeviceInformations[index].connHandle,
						  	  	  	  	  	  	  usedDeviceInformations[index].servicesHandle.CSCServicehandle.P2PServiceHandle,
												  usedDeviceInformations[index].servicesHandle.CSCServicehandle.P2PServiceEndHandle);
				  break;
	    	  case POWER_SENSOR:
	    		  APP_DBG_MSG("* GATT : Discover P2P Characteristics power sensor\n");
				  aci_gatt_disc_all_char_of_service(usedDeviceInformations[index].connHandle,
												  usedDeviceInformations[index].servicesHandle.PowerServicehandle.P2PServiceHandle,
												  usedDeviceInformations[index].servicesHandle.PowerServicehandle.P2PServiceEndHandle);
				  break;
	    	  case TRAINER:
				  APP_DBG_MSG("* GATT : Discover P2P Characteristics trainer\n");
				  aci_gatt_disc_all_char_of_service(usedDeviceInformations[index].connHandle,
						  	  	  	  	  	  	  usedDeviceInformations[index].servicesHandle.PowerServicehandle.P2PServiceHandle,
												  usedDeviceInformations[index].servicesHandle.PowerServicehandle.P2PServiceEndHandle);
				  break;
	    	  case SHIMANO_SENSOR:
				  APP_DBG_MSG("* GATT : Discover P2P Characteristics Shimano\n");
				  aci_gatt_disc_all_char_of_service(usedDeviceInformations[index].connHandle,
												  usedDeviceInformations[index].servicesHandle.ShimanoServicehandle.P2PServiceHandle,
												  usedDeviceInformations[index].servicesHandle.ShimanoServicehandle.P2PServiceEndHandle);
				  break;
	    	  default:
	    		  APP_DBG_MSG("* TYPE DE CAPTEUR NON RECONNU\n");
	    		  break;
	    	  }


	        break;

	      case APP_BLE_DISCOVER_NOTIFICATION_CHAR_DESC:
	        APP_DBG_MSG("* GATT : Discover Descriptor of Rx - Notification Characteritic\n");
	        aci_gatt_disc_all_char_desc(usedDeviceInformations[index].connHandle,
										usedDeviceInformations[index].servicesHandle.P2PNotificationCharHdle,
										usedDeviceInformations[index].servicesHandle.P2PNotificationCharHdle+2);

	        break;
	      case APP_BLE_READ_CHARACS:
	        APP_DBG_MSG("* GATT : Discover Reading characs - \n");
	        aci_gatt_read_char_value(aP2PClientContext[index].connHandle,
	        	                      aP2PClientContext[index].P2PReadCharHdle);

	        aP2PClientContext[index].state = APP_BLE_DISCOVER_NOTIFICATION_CHAR_DESC;
	        break;
	      case APP_BLE_ENABLE_NOTIFICATION_DESC:
	        APP_DBG_MSG("* GATT : Enable Server Notification\n");
	        aci_gatt_write_char_desc(usedDeviceInformations[index].connHandle,
	        						 usedDeviceInformations[index].servicesHandle.P2PNotificationDescHandle,
	                                 2,
	                                 (uint8_t *)&enable);

	        usedDeviceInformations[index].state = APP_BLE_CONNECTED_CLIENT;
	        BSP_LED_Off(LED_RED);

	        break;
	      case APP_BLE_DISABLE_NOTIFICATION_DESC :
	        APP_DBG_MSG("* GATT : Disable Server Notification\n");
	        aci_gatt_write_char_desc(aP2PClientContext[index].connHandle,
	                                 aP2PClientContext[index].P2PNotificationDescHandle,
	                                 2,
	                                 (uint8_t *)&disable);

	        aP2PClientContext[index].state = APP_BLE_CONNECTED_CLIENT;

	        break;
	      default:
	        break;
	    }
	    index++;
	  }
	  return;
}

static SVCCTL_EvtAckStatus_t Client_Event_Handler(void *Event)
{
    /* USER CODE BEGIN Client_Event_Handler_1 */

    /* USER CODE END Client_Event_Handler_1 */
    SVCCTL_EvtAckStatus_t return_value;
    hci_event_pckt *event_pckt;
    evt_blue_aci *blue_evt;
    P2P_Client_App_Notification_evt_t Notification;
    return_value = SVCCTL_EvtNotAck;
    event_pckt = (hci_event_pckt *)(((hci_uart_pckt*)Event)->data);


    switch(event_pckt->evt)
    {
        case EVT_VENDOR:
        {
            /* USER CODE BEGIN EVT_VENDOR */

            /* USER CODE END EVT_VENDOR */
            blue_evt = (evt_blue_aci*)event_pckt->data;
            switch(blue_evt->ecode)
            {
            /* USER CODE BEGIN ecode */

            /* USER CODE END ecode */
                case EVT_BLUE_ATT_READ_BY_GROUP_TYPE_RESP:
                {
                    /* USER CODE BEGIN EVT_BLUE_ATT_READ_BY_GROUP_TYPE_RESP */
                    bool uuid_bit_format = 0;
                	struct settings readSettings;
                	readFlash((uint8_t*)&readSettings);

                    /* USER CODE END EVT_BLUE_ATT_READ_BY_GROUP_TYPE_RESP */
                    aci_att_read_by_group_type_resp_event_rp0 *pr = (void*)blue_evt->data;
                    uint8_t numServ, i, idx;
                    uint16_t uuid, handle;

                    uint8_t index;
                    handle = pr->Connection_Handle;
                    index = 0;
                    while((index < BLE_CFG_CLT_MAX_NBR_CB) &&
                            (usedDeviceInformations[index].state != APP_BLE_IDLE) &&
                        (usedDeviceInformations[index].connHandle != handle))
                    {
                        index++;
                    }
                    //Debut format 128bit ou 16bit
                    	switch(pr->Attribute_Data_Length)
                    	{
                    	case 20:
                    		uuid_bit_format = 1;
                    		break;
                    	case 6:
                    		uuid_bit_format = 0;
                    		break;
                    	default:

                    	break;
                    	}
                    //Fin format 128 bit ou 16 bit

                    if(index < BLE_CFG_CLT_MAX_NBR_CB)
                    {
                    	usedDeviceInformations[index].connHandle= handle;
                        numServ = (pr->Data_Length) / pr->Attribute_Data_Length;
                        /* the event data will be
                         * 2bytes start handle
                         * 2bytes end handle
                         * 2 or 16 bytes data
                         * we are intersted only if the UUID is 16 bit.
                         * So check if the data length is 6
                         * we are intersted only if the UUID is 128 bit.
                         * So check if the data length is 20
                         */

                                if (uuid_bit_format==1){idx = 16;}
                                else if (uuid_bit_format==0){idx = 4;}

                                if (pr->Attribute_Data_Length == 20 || pr->Attribute_Data_Length == 6)
                                {

                                for (i=0; i<numServ; i++)
                                {
                                    uuid = UNPACK_2_BYTE_PARAMETER(&pr->Attribute_Data_List[idx]);
                                    switch(uuid)
                                    {
                                    	case(CYCLING_SPEED_CADENCE_SERVICE_UUID ):
										{
                                    		usedDeviceInformations[index].supportedDataType.cadence = true;
                                    		usedDeviceInformations[index].supportedDataType.speed = true;
#if(CFG_DEBUG_APP_TRACE != 0)
											APP_DBG_MSG("-- GATT : CSC_SERVICE_UUID FOUND - connection handle 0x%x \n", aP2PClientContext[index].connHandle);
#endif

												usedDeviceInformations[index].servicesHandle.CSCServicehandle.P2PServiceHandle = UNPACK_2_BYTE_PARAMETER(&pr->Attribute_Data_List[idx-4]);
												usedDeviceInformations[index].servicesHandle.CSCServicehandle.P2PServiceEndHandle = UNPACK_2_BYTE_PARAMETER (&pr->Attribute_Data_List[idx-2]);

											break;
										}
										case(CYCLING_POWER_SERVICE_UUID):
										{
											usedDeviceInformations[index].supportedDataType.power = true;

#if(CFG_DEBUG_APP_TRACE != 0)
											APP_DBG_MSG("-- GATT : CYCLING_POWER_SERVICE_UUID FOUND - connection handle 0x%x \n", aP2PClientContext[index].connHandle);
#endif

												usedDeviceInformations[index].servicesHandle.PowerServicehandle.P2PServiceHandle = UNPACK_2_BYTE_PARAMETER(&pr->Attribute_Data_List[idx-4]);
												usedDeviceInformations[index].servicesHandle.PowerServicehandle.P2PServiceEndHandle = UNPACK_2_BYTE_PARAMETER (&pr->Attribute_Data_List[idx-2]);
											break;
										}
										case(BATTERY_SERVICE_UUID):
										{
											usedDeviceInformations[index].supportedDataType.battery = true;
											break;
										}
                                    	case(SHIMANO_SERVICE_UUID ):
                                    	{
#if(CFG_DEBUG_APP_TRACE != 0)
											APP_DBG_MSG("-- GATT : SENSOR_SERVICE_UUID FOUND - connection handle 0x%x \n", aP2PClientContext[index].connHandle);
#endif
											usedDeviceInformations[index].supportedDataType.gear = true;

												usedDeviceInformations[index].servicesHandle.ShimanoServicehandle.P2PServiceHandle = UNPACK_2_BYTE_PARAMETER(&pr->Attribute_Data_List[idx-16]);
												usedDeviceInformations[index].servicesHandle.ShimanoServicehandle.P2PServiceEndHandle = UNPACK_2_BYTE_PARAMETER (&pr->Attribute_Data_List[idx-14]);
                                    		break;
                                    	}
                                    	default:
                                    		break;
                                    }
                                    idx += 6;
                                }
								if (usedDeviceInformations[index].supportedDataType.cadence == true &&
										usedDeviceInformations[index].supportedDataType.speed == true &&
										usedDeviceInformations[index].supportedDataType.power == true)
								{
									usedDeviceInformations[index].sensorType = TRAINER;
									usedDeviceInformations[index].state = APP_BLE_DISCOVER_CHARACS ;
									usedDeviceInformations[index].sensor_evt_type = P2P_NOTIFICATION_CP_RECEIVED_EVT;
								}
								else if(usedDeviceInformations[index].supportedDataType.cadence == true &&
										usedDeviceInformations[index].supportedDataType.speed == true &&
										usedDeviceInformations[index].supportedDataType.power == false)
								{
									usedDeviceInformations[index].sensorType = CSC_SENSOR;
									usedDeviceInformations[index].state = APP_BLE_DISCOVER_CHARACS ;
									usedDeviceInformations[index].sensor_evt_type = P2P_NOTIFICATION_CSC_RECEIVED_EVT;
								}
								else if(usedDeviceInformations[index].supportedDataType.cadence == false &&
										usedDeviceInformations[index].supportedDataType.speed == false &&
										usedDeviceInformations[index].supportedDataType.power == true)
								{
									usedDeviceInformations[index].sensorType = POWER_SENSOR;
									usedDeviceInformations[index].state = APP_BLE_DISCOVER_CHARACS ;
									usedDeviceInformations[index].sensor_evt_type = P2P_NOTIFICATION_CP_RECEIVED_EVT;
								}
								else if(usedDeviceInformations[index].supportedDataType.gear == true){

									usedDeviceInformations[index].sensorType = SHIMANO_SENSOR;
									usedDeviceInformations[index].state = APP_BLE_DISCOVER_CHARACS ;
									usedDeviceInformations[index].sensor_evt_type = P2P_NOTIFICATION_SHIMANO_RECEIVED_EVT;
								}
								else{
									usedDeviceInformations[index].sensorType = OTHER;
								}
                            }
                        }
                    }
                    break;//ok

                case EVT_BLUE_ATT_READ_BY_TYPE_RESP:
                {
                    /* USER CODE BEGIN EVT_BLUE_ATT_READ_BY_TYPE_RESP */
                	bool uuid_format_char = 0;
                    /* USER CODE END EVT_BLUE_ATT_READ_BY_TYPE_RESP */
                    aci_att_read_by_type_resp_event_rp0 *pr = (void*)blue_evt->data;

                    uint8_t idx;
                    uint16_t uuid, handle;

                    /* the event data will be
                     * 2 bytes start handle
                     * 1 byte char properties
                     * 2 bytes handle
                     * 2 or 16 bytes data
                     */

                    uint8_t index;

                    index = 0;

                    while((index < BLE_CFG_CLT_MAX_NBR_CB) &&
                            (usedDeviceInformations[index].connHandle != pr->Connection_Handle))
                    {
                        index++;
                    }


                    if(index < BLE_CFG_CLT_MAX_NBR_CB)
                    {





                        //Debut format 128bit ou 16bit
                        	switch(pr->Handle_Value_Pair_Length)
                        	{
                        	case 21:
                        		uuid_format_char = 1;
                        		break;
                        	case 7:
                        		uuid_format_char = 0;
                        		break;
                        	default:

                        	break;
                        	}
                        //Fin format 128 bit ou 16 bit

                          /* we are not just interested in only 16 bit UUIDs */
                          if (uuid_format_char == 1){idx=17;}
                          else if (uuid_format_char==0){idx=5;}


                        if(pr->Handle_Value_Pair_Length == 21 || pr->Handle_Value_Pair_Length == 7)

                        {

                        	pr->Data_Length -= 1;
                            while(pr->Data_Length > 0)
                            {
                                uuid = UNPACK_2_BYTE_PARAMETER(&pr->Handle_Value_Pair_Data[idx]);
                                /* store the characteristic handle not the attribute handle */

                                if(uuid_format_char==1){handle = UNPACK_2_BYTE_PARAMETER(&pr->Handle_Value_Pair_Data[idx-14]);}
                                else if(uuid_format_char==0){handle = UNPACK_2_BYTE_PARAMETER(&pr->Handle_Value_Pair_Data[idx-2]);}

                                if(uuid == CYCLING_SPEED_CADENCE_MEASUREMENT_CHAR_UUID && usedDeviceInformations[index].sensorType == CSC_SENSOR)
                                {
#if(CFG_DEBUG_APP_TRACE != 0)
                                	  APP_DBG_MSG("-- GATT : CSC_NOTIFY_CHAR_UUID FOUND  - connection handle 0x%x\n", aP2PClientContext[index].connHandle);
#endif
                                	  usedDeviceInformations[index].servicesHandle.P2PNotificationCharHdle = handle;
                                	  usedDeviceInformations[index].state = APP_BLE_DISCOVER_NOTIFICATION_CHAR_DESC;
                                }
                                if(uuid == SHIMANO_CHAR_UUID && usedDeviceInformations[index].sensorType == SHIMANO_SENSOR){
#if(CFG_DEBUG_APP_TRACE != 0)
                                	  APP_DBG_MSG("-- GATT : SHIMANO_NOTIFY_CHAR_UUID FOUND  - connection handle 0x%x\n", aP2PClientContext[index].connHandle);
#endif
                                	  usedDeviceInformations[index].servicesHandle.P2PNotificationCharHdle = handle;
                                	  usedDeviceInformations[index].state = APP_BLE_DISCOVER_NOTIFICATION_CHAR_DESC;

                                }
                                /*else if (uuid == CYCLING_SPEED_CADENCE_FEATURE_CHAR_UUID && scannedDevicesPackage.scannedDevicesList[indexOfInScannedDeviceList].supportedDataType.speed == true)
                                {
#if(CFG_DEBUG_APP_TRACE != 0)
                                	APP_DBG_MSG("-- GATT : SENSOR_READ_CHAR_UUID FOUND  - connection handle 0x%x\n", aP2PClientContext[index].connHandle);
#endif

                                	aP2PClientContext[index].state = APP_BLE_READ_CHARACS;
									aP2PClientContext[index].P2PReadCharHdle = handle;
									aP2PClientContext[index].P2PcurrentCharBeingRead = CYCLING_SPEED_CADENCE_FEATURE_CHAR_UUID;

                                }*/

                                if(uuid == CYCLING_POWER_MEASUREMENT_CHAR_UUID && (usedDeviceInformations[index].sensorType == TRAINER || POWER_SENSOR))
								{
#if(CFG_DEBUG_APP_TRACE != 0)
									  APP_DBG_MSG("-- GATT : POWER_NOTIFY_CHAR_UUID FOUND  - connection handle 0x%x\n", aP2PClientContext[index].connHandle);
#endif
									  usedDeviceInformations[index].servicesHandle.P2PNotificationCharHdle = handle;
									  usedDeviceInformations[index].state = APP_BLE_DISCOVER_NOTIFICATION_CHAR_DESC;
								}

                                if(uuid_format_char==1){
                                    pr->Data_Length -= 21;
                                    idx += 21;
                                }
                                else if (uuid_format_char==0){
                                    pr->Data_Length -= 7;;
                                    idx += 7;;
                                }


                            }
                        }
                    }
                }
                break; //ok


                case EVT_BLUE_ATT_FIND_INFORMATION_RESP:
                {
                    /* USER CODE BEGIN EVT_BLUE_ATT_FIND_INFORMATION_RESP */

                    /* USER CODE END EVT_BLUE_ATT_FIND_INFORMATION_RESP */
                    aci_att_find_info_resp_event_rp0 *pr = (void*)blue_evt->data;

                    uint8_t numDesc, idx, i;
                    uint16_t uuid, handle;

                    /*
                     * event data will be of the format
                     * 2 bytes handle
                     * 2 bytes UUID
                     */

                    uint8_t index;

                    index = 0;
                    while((index < BLE_CFG_CLT_MAX_NBR_CB) &&
                            (usedDeviceInformations[index].connHandle != pr->Connection_Handle))

                        index++;

                    if(index < BLE_CFG_CLT_MAX_NBR_CB)
                    {

                        numDesc = (pr->Event_Data_Length) / 4;
                        /* we are interested only in 16 bit UUIDs */
                        idx = 0;
                        if (pr->Format == UUID_TYPE_16) //À REGARDER-----------------------------------------------------------
                        {
                            for (i=0; i<numDesc; i++)
                            {
                                handle = UNPACK_2_BYTE_PARAMETER(&pr->Handle_UUID_Pair[idx]);
                                uuid = UNPACK_2_BYTE_PARAMETER(&pr->Handle_UUID_Pair[idx+2]);

                                if(uuid == CLIENT_CHAR_CONFIG_DESCRIPTOR_UUID)
                                {
#if(CFG_DEBUG_APP_TRACE != 0)
                                    APP_DBG_MSG("-- GATT : CLIENT_CHAR_CONFIG_DESCRIPTOR_UUID- connection handle 0x%x\n", aP2PClientContext[index].connHandle);
#endif
                                    if( usedDeviceInformations[index].state == APP_BLE_DISCOVER_NOTIFICATION_CHAR_DESC)
                                    {

                                    	usedDeviceInformations[index].servicesHandle.P2PNotificationDescHandle = handle;
                                        usedDeviceInformations[index].state = APP_BLE_ENABLE_NOTIFICATION_DESC;

                                    }
                                }
                                idx += 4;
                            }
                        }
                    }
                }
                break; /*EVT_BLUE_ATT_FIND_INFORMATION_RESP*/ //ok

                case EVT_BLUE_GATT_NOTIFICATION:
                {
                    /* USER CODE BEGIN EVT_BLUE_GATT_NOTIFICATION */

                    /* USER CODE END EVT_BLUE_GATT_NOTIFICATION */
                    aci_gatt_notification_event_rp0 *pr = (void*)blue_evt->data;
                    uint8_t index;

                    index = 0;
                    while((index < BLE_CFG_CLT_MAX_NBR_CB) &&
                            (usedDeviceInformations[index].connHandle != pr->Connection_Handle))
                        index++;

                    if(index < BLE_CFG_CLT_MAX_NBR_CB)
                    {

                        if ( (pr->Attribute_Handle == usedDeviceInformations[index].servicesHandle.P2PNotificationCharHdle))
                        {
#if(CFG_DEBUG_APP_TRACE != 0)

#endif

                            Notification.P2P_Client_Evt_Opcode = usedDeviceInformations[index].sensor_evt_type;
                            Notification.DataTransfered.Length = pr->Attribute_Value_Length;
                            Notification.DataTransfered.pPayload = pr->Attribute_Value;

                            P2P_Client_App_Notification(&Notification);



                            /* INFORM APPLICATION BUTTON IS PUSHED BY END DEVICE */

                        }
                    }
                }
                break;/* end EVT_BLUE_GATT_NOTIFICATION */ //ok

                case EVT_BLUE_ATT_READ_RESP:
                {

                	aci_att_read_resp_event_rp0 *pr = (void*)blue_evt->data;
                    uint8_t index;
                    int sensorIndex;

                    index = 0;
                    sensorIndex = getCorrespondingIndex(SENSOR_NAME);

                    while((index < BLE_CFG_CLT_MAX_NBR_CB) &&
                            (aP2PClientContext[index].connHandle != pr->Connection_Handle))
                        index++;

                    if(aP2PClientContext[index].P2PcurrentCharBeingRead == CYCLING_SPEED_CADENCE_FEATURE_CHAR_UUID){
                        printf("\n\rvalue is");
                        for (int i = 0; i<pr->Event_Data_Length; i++){
                            printf("%x", pr->Attribute_Value[i]);
                        }
                        printf("\n\r");

                        scannedDevicesPackage.scannedDevicesList[sensorIndex].supportedDataType.cadence = pr->Attribute_Value[0] & 0x2;
                        scannedDevicesPackage.scannedDevicesList[sensorIndex].supportedDataType.speed = pr->Attribute_Value[0] & 0x1;
                    }

                	break; //ok
                }
                case EVT_BLUE_GATT_PROCEDURE_COMPLETE:
                {
                    /* USER CODE BEGIN EVT_BLUE_GATT_PROCEDURE_COMPLETE */

                    /* USER CODE END EVT_BLUE_GATT_PROCEDURE_COMPLETE */
                    aci_gatt_proc_complete_event_rp0 *pr = (void*)blue_evt->data;
#if(CFG_DEBUG_APP_TRACE != 0)
                    APP_DBG_MSG("-- GATT : EVT_BLUE_GATT_PROCEDURE_COMPLETE \n");
                    APP_DBG_MSG("\n");
#endif
                    uint8_t index;
                    index = 0;
                    while((index < BLE_CFG_CLT_MAX_NBR_CB) &&
                            (usedDeviceInformations[index].connHandle != pr->Connection_Handle))
                        index++;

                    if(index < BLE_CFG_CLT_MAX_NBR_CB)
                    {
                        UTIL_SEQ_SetTask(  1<<CFG_TASK_SEARCH_SERVICE_ID, CFG_SCH_PRIO_0 );
                    }
                }
                break; /*EVT_BLUE_GATT_PROCEDURE_COMPLETE*/ //ok
                default:
                    /* USER CODE BEGIN ecode_default */

                    /* USER CODE END ecode_default */
                    break;
                }
            }

            break; /* HCI_EVT_VENDOR_SPECIFIC */

        default:
            /* USER CODE BEGIN evt_default */

            /* USER CODE END evt_default */
            break;
        }
/* USER CODE BEGIN Client_Event_Handler_2 */

/* USER CODE END Client_Event_Handler_2 */
        return(return_value);
    }/* end BLE_CTRL_Event_Acknowledged_Status_t */

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */

/* USER CODE END FD_LOCAL_FUNCTIONS */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
