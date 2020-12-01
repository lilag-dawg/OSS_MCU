/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    app_ble.c
 * @author  MCD Application Team
 * @brief   BLE Application
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
#include "app_common.h"

#include "dbg_trace.h"

#include "ble.h"
#include "tl.h"
#include "app_ble.h"

#include "stm32_seq.h"
#include "shci.h"
#include "stm32_lpm.h"
#include "otp.h"

#include "p2p_routeur_app.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include <string.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/**
 * security parameters structure
 */
typedef struct _tSecurityParams
{
  /**
   * IO capability of the device
   */
  uint8_t ioCapability;

  /**
   * Authentication requirement of the device
   * Man In the Middle protection required?
   */
  uint8_t mitm_mode;

  /**
   * bonding mode of the device
   */
  uint8_t bonding_mode;

  /**
   * Flag to tell whether OOB data has
   * to be used during the pairing process
   */
  uint8_t OOB_Data_Present;

  /**
   * OOB data to be used in the pairing process if
   * OOB_Data_Present is set to TRUE
   */
  uint8_t OOB_Data[16];

  /**
   * this variable indicates whether to use a fixed pin
   * during the pairing process or a passkey has to be
   * requested to the application during the pairing process
   * 0 implies use fixed pin and 1 implies request for passkey
   */
  uint8_t Use_Fixed_Pin;

  /**
   * minimum encryption key size requirement
   */
  uint8_t encryptionKeySizeMin;

  /**
   * maximum encryption key size requirement
   */
  uint8_t encryptionKeySizeMax;

  /**
   * fixed pin to be used in the pairing process if
   * Use_Fixed_Pin is set to 1
   */
  uint32_t Fixed_Pin;

  /**
   * this flag indicates whether the host has to initiate
   * the security, wait for pairing or does not have any security
   * requirements.\n
   * 0x00 : no security required
   * 0x01 : host should initiate security by sending the slave security
   *        request command
   * 0x02 : host need not send the clave security request but it
   * has to wait for paiirng to complete before doing any other
   * processing
   */
  uint8_t initiateSecurity;
} tSecurityParams;

/**
 * global context
 * contains the variables common to all
 * services
 */
typedef struct _tBLEProfileGlobalContext
{

  /**
   * security requirements of the host
   */
  tSecurityParams bleSecurityParam;

  /**
   * gap service handle
   */
  uint16_t gapServiceHandle;

  /**
   * device name characteristic handle
   */
  uint16_t devNameCharHandle;

  /**
   * appearance characteristic handle
   */
  uint16_t appearanceCharHandle;

  /**
   * connection handle of the current active connection
   * When not in connection, the handle is set to 0xFFFF
   */
  uint16_t connectionHandle[CFG_MAX_CONNECTION];

  /**
   * length of the UUID list to be used while advertising
   */
  uint8_t advtServUUIDlen;

  /**
   * the UUID list to be used while advertising
   */
  uint8_t advtServUUID[100];

} BleGlobalContext_t;

typedef struct
{
  BleGlobalContext_t BleApplicationContext_legacy;
  /**
   * used to identify the GAP State
   */
  APP_BLE_ConnStatus_t SmartPhone_Connection_Status;

  /**
   * used to identify the GAP State
   */
  APP_BLE_ConnStatus_t EndDevice_Connection_Status[4];
  /**
   * connection handle with the Central connection (Smart Phone)
   * When not in connection, the handle is set to 0xFFFF
   */
  uint16_t connectionHandleCentral;
  /**
   * connection handle with the Server 1 connection (End Device 1)
   * When not in connection, the handle is set to 0xFFFF
   */
  uint16_t connectionHandleEndDevice1;

#if (CFG_P2P_DEMO_MULTI != 0)
/* USER CODE BEGIN connectionHandleEndDevice_Multi */
  /**
   * connection handle with the Server 2 connection (End Device 2)
   * When not in connection, the handle is set to 0xFFFF
   */
  uint16_t connectionHandleEndDevice2;
  /**
   * connection handle with the Server 3 connection (End Device 3)
   * When not in connection, the handle is set to 0xFFFF
   */
  uint16_t connectionHandleEndDevice3;
  /**
   * connection handle with the Server 4 connection (End Device 4)
   * When not in connection, the handle is set to 0xFFFF
   */
  uint16_t connectionHandleEndDevice4;
/* USER CODE END connectionHandleEndDevice_Multi */
#endif

  /**
   * used when doing advertising to find end device 1
   */
  uint8_t EndDevice1Found;

#if (CFG_P2P_DEMO_MULTI != 0)
/* USER CODE BEGIN EndDeviceFound_Multi */
  /**
   * used when doing advertising to find end device 2
   */
  uint8_t EndDevice2Found;
  /**
   * used when doing advertising to find end device 3
   */
  uint8_t EndDevice3Found;
  /**
   * used when doing advertising to find end device 4
   */
  uint8_t EndDevice4Found;
/* USER CODE END EndDeviceFound_Multi */
#endif

} BleApplicationContext_t;


typedef struct
{
  uint16_t Connection_Handle;
  uint8_t  Identifier;
  uint16_t L2CAP_Length;
  uint16_t Interval_Min;
  uint16_t Interval_Max;
  uint16_t Slave_Latency;
  uint16_t Timeout_Multiplier;
} APP_BLE_p2p_Conn_Update_req_t;

typedef enum
{
  P2P_SERVER1_CONN_HANDLE_EVT,
  P2P_SERVER1_DISCON_HANDLE_EVT,
  SMART_PHONE1_CONN_HANDLE_EVT,
  SMART_PHONE1_DISCON_HANDLE_EVT,
#if (CFG_P2P_DEMO_MULTI != 0)
/* USER CODE BEGIN P2P_SERVER_CONN_HANDLE_EVT_Multi */
  P2P_SERVER2_CONN_HANDLE_EVT,
  P2P_SERVER3_CONN_HANDLE_EVT,
  P2P_SERVER4_CONN_HANDLE_EVT,
/* USER CODE END P2P_SERVER_CONN_HANDLE_EVT_Multi */
/* USER CODE BEGIN P2P_SERVER_DISCON_HANDLE_EVT_Multi */
  P2P_SERVER2_DISCON_HANDLE_EVT,
  P2P_SERVER3_DISCON_HANDLE_EVT,
  P2P_SERVER4_DISCON_HANDLE_EVT,
/* USER CODE END P2P_SERVER_DISCON_HANDLE_EVT_Multi */
#endif

} P2P_Opcode_Notification_evt_t;

typedef struct
{
  P2P_Opcode_Notification_evt_t P2P_Evt_Opcode;
  uint16_t ConnectionHandle;

} P2P_ConnHandle_Not_evt_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/
#define APPBLE_GAP_DEVICE_NAME_LENGTH 7

#define BD_ADDR_SIZE_LOCAL    6

/* USER CODE BEGIN PD */
#define MAX_NMBR_DEVICES      127

//struct DeviceInformations_t *DeviceFound[MAX_NMBR_DEVICES];
 ScannedDevicesPackage_t scannedDevicesPackage;
 UsedDeviceInformations_t usedDeviceInformations[4];
 BikeDataInformation_t bikeDataInformation;

 uint8_t macRidesense[6] = {21,42,96,10,26,236};
 uint8_t macShimano[6] = {159,110,155,254,115,235};
 uint8_t macTackx[6] = {48,94,167,136,151,216};
 uint8_t macFlux[6] = {235,237,146,151,168,238};

/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
PLACE_IN_SECTION("MB_MEM1") ALIGN(4) static TL_CmdPacket_t BleCmdBuffer;

static const uint8_t M_bd_addr[BD_ADDR_SIZE_LOCAL] =
    {
        (uint8_t)((CFG_ADV_BD_ADDRESS & 0x0000000000FF)),
        (uint8_t)((CFG_ADV_BD_ADDRESS & 0x00000000FF00) >> 8),
        (uint8_t)((CFG_ADV_BD_ADDRESS & 0x000000FF0000) >> 16),
        (uint8_t)((CFG_ADV_BD_ADDRESS & 0x0000FF000000) >> 24),
        (uint8_t)((CFG_ADV_BD_ADDRESS & 0x00FF00000000) >> 32),
        (uint8_t)((CFG_ADV_BD_ADDRESS & 0xFF0000000000) >> 40)
    };

static uint8_t bd_addr_udn[BD_ADDR_SIZE_LOCAL];

/**
*   Identity root key used to derive LTK and CSRK
*/
static const uint8_t BLE_CFG_IR_VALUE[16] = CFG_BLE_IRK;

/**
* Encryption root key used to derive LTK and CSRK
*/
static const uint8_t BLE_CFG_ER_VALUE[16] = CFG_BLE_ERK;

/**
 * BD Address of SERVER1 & SERVER 2 - to be connected once discovered
 */
tBDAddr P2P_SERVER1_BDADDR;
#if (CFG_P2P_DEMO_MULTI != 0)
/* USER CODE BEGIN P2P_SERVER_BDADDR_Multi */
tBDAddr P2P_SERVER2_BDADDR;
tBDAddr P2P_SERVER3_BDADDR;
tBDAddr P2P_SERVER4_BDADDR;
tBDAddr P2P_SERVER5_BDADDR;
tBDAddr P2P_SERVER6_BDADDR;
/* USER CODE END P2P_SERVER_BDADDR_Multi */
#endif
tBDAddr SERVER_REMOTE_BDADDR;

/**
 * Advertising Data
 */
static char local_name[] = { AD_TYPE_COMPLETE_LOCAL_NAME,'O','S','S','-','0','1'};
uint8_t manuf_data[14] = {
    sizeof(manuf_data)-1, AD_TYPE_MANUFACTURER_SPECIFIC_DATA,
    0x01/*SKD version */,
    CFG_DEV_ID_P2P_ROUTER /* STM32WB - P2P Router*/,
    0x00 /* GROUP A Feature  */,
    0x00 /* GROUP A Feature */,
    0x00 /* GROUP B Feature */,
    0x00 /* GROUP B Feature */,
    0x00, /* BLE MAC start -MSB */
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, /* BLE MAC stop */
};

P2P_ConnHandle_Not_evt_t handleNotification;

PLACE_IN_SECTION("BLE_APP_CONTEXT") static BleApplicationContext_t BleApplicationContext;
PLACE_IN_SECTION("BLE_APP_CONTEXT") APP_BLE_p2p_Conn_Update_req_t APP_BLE_p2p_Conn_Update_req;

uint16_t connection_handle;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void BLE_UserEvtRx( void * pPayload );
static void BLE_StatusNot( HCI_TL_CmdStatus_t status );
static void Ble_Tl_Init( void );
static void Ble_Hci_Gap_Gatt_Init(void);
static const uint8_t* BleGetBdAddress( void );
static void Scan_Request( void );
static void Evt_Notification( P2P_ConnHandle_Not_evt_t *pNotification );
static void ConnReq1( void );
static void Adv_Request( void );
/* USER CODE BEGIN PFP */
#if (CFG_P2P_DEMO_MULTI != 0)  
static void ConnReq2( void );
static void ConnReq3( void );
static void ConnReq4( void );
#endif
/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
void APP_BLE_Init( void )
{
/* USER CODE BEGIN APP_BLE_Init_1 */

/* USER CODE END APP_BLE_Init_1 */
  SHCI_C2_Ble_Init_Cmd_Packet_t ble_init_cmd_packet =
  {
    {{0,0,0}},                          /**< Header unused */
    {0,                                 /** pBleBufferAddress not used */
    0,                                  /** BleBufferSize not used */
    CFG_BLE_NUM_GATT_ATTRIBUTES,
    CFG_BLE_NUM_GATT_SERVICES,
    CFG_BLE_ATT_VALUE_ARRAY_SIZE,
    CFG_BLE_NUM_LINK,
    CFG_BLE_DATA_LENGTH_EXTENSION,
    CFG_BLE_PREPARE_WRITE_LIST_SIZE,
    CFG_BLE_MBLOCK_COUNT,
    CFG_BLE_MAX_ATT_MTU,
    CFG_BLE_SLAVE_SCA,
    CFG_BLE_MASTER_SCA,
    CFG_BLE_LSE_SOURCE,
    CFG_BLE_MAX_CONN_EVENT_LENGTH,
    CFG_BLE_HSE_STARTUP_TIME,
    CFG_BLE_VITERBI_MODE,
    CFG_BLE_LL_ONLY,
    0}
  };

  /**
   * Initialize Ble Transport Layer
   */
  Ble_Tl_Init( );

  /**
   * Do not allow standby in the application
   */
  UTIL_LPM_SetOffMode(1 << CFG_LPM_APP_BLE, UTIL_LPM_DISABLE);

/**
   * Register the hci transport layer to handle BLE User Asynchronous Events
   */
 UTIL_SEQ_RegTask( 1<<CFG_TASK_HCI_ASYNCH_EVT_ID, UTIL_SEQ_RFU, hci_user_evt_proc);

  /**
   * Starts the BLE Stack on CPU2
   */
  SHCI_C2_BLE_Init( &ble_init_cmd_packet );

  /**
   * Initialization of HCI & GATT & GAP layer
   */
  Ble_Hci_Gap_Gatt_Init();

  /**
   * Initialization of the BLE Services
   */
  SVCCTL_Init();

  /**
   * From here, all initialization are BLE application specific
   */
  UTIL_SEQ_RegTask( 1<<CFG_TASK_START_SCAN_ID, UTIL_SEQ_RFU, Scan_Request);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_CONN_DEV_1_ID, UTIL_SEQ_RFU, ConnReq1);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_START_ADV_ID, UTIL_SEQ_RFU, Adv_Request);
#if (CFG_P2P_DEMO_MULTI != 0)
/* USER CODE BEGIN SCH_RegTask_Multi */
  UTIL_SEQ_RegTask( 1<<CFG_TASK_CONN_DEV_2_ID, UTIL_SEQ_RFU, ConnReq2);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_CONN_DEV_3_ID, UTIL_SEQ_RFU, ConnReq3);
  UTIL_SEQ_RegTask( 1<<CFG_TASK_CONN_DEV_4_ID, UTIL_SEQ_RFU, ConnReq4);

/* USER CODE END SCH_RegTask_Multi */
#endif

  /**
   * Initialization of the BLE App Context
   */
  BleApplicationContext.SmartPhone_Connection_Status = APP_BLE_IDLE;
  BleApplicationContext.EndDevice_Connection_Status[0] = APP_BLE_IDLE;
  BleApplicationContext.EndDevice1Found = 0x00;

#if (CFG_P2P_DEMO_MULTI != 0)
/* USER CODE BEGIN Connection_Status_Multi */
  BleApplicationContext.EndDevice_Connection_Status[1] = APP_BLE_IDLE;
  BleApplicationContext.EndDevice_Connection_Status[2] = APP_BLE_IDLE;
  BleApplicationContext.EndDevice_Connection_Status[3] = APP_BLE_IDLE;
/* USER CODE END Connection_Status_Multi */
/* USER CODE BEGIN EndDeviceFound_Multi_Init */
  BleApplicationContext.EndDevice2Found = 0x00;
  BleApplicationContext.EndDevice3Found = 0x00;
  BleApplicationContext.EndDevice4Found = 0x00;
/* USER CODE END EndDeviceFound_Multi_Init */
#endif
/**
 * Initialize P2P Routeur Application
 */
  P2PR_APP_Init();

  /**
   * Start scanning
   */
  UTIL_SEQ_SetTask(1 << CFG_TASK_START_ADV_ID, CFG_SCH_PRIO_0);
/* USER CODE BEGIN APP_BLE_Init_2 */

  UTIL_SEQ_SetTask(1 << CFG_TASK_START_SCAN_ID, CFG_SCH_PRIO_1);

  //reset list of scanned devices
  memset(&scannedDevicesPackage, 0 , sizeof(scannedDevicesPackage));

  //reset used device informations
  memset(&usedDeviceInformations, 0 , sizeof(usedDeviceInformations));

  //reset calibration parameters
  memset(&bikeDataInformation, 0 , sizeof(bikeDataInformation));


  //reset flash
  //settings_t settingsToWrite;
  //memset(&settingsToWrite, 0 , sizeof(settingsToWrite));

  //initilase sensor hardcode
 //strcpy(settingsToWrite.sensors[0].name,"	Tacx Vortex 18043");
 //memcpy(settingsToWrite.sensors[0].macAddress, macTackx, sizeof(settingsToWrite.sensors[0].macAddress));
//
 //strcpy(settingsToWrite.sensors[1].name,"	Ridesense");
 //memcpy(settingsToWrite.sensors[1].macAddress, macRidesense, sizeof(settingsToWrite.sensors[1].macAddress));
//
 //strcpy(settingsToWrite.sensors[2].name,"	EWWU111");
 //memcpy(settingsToWrite.sensors[2].macAddress, macShimano, sizeof(settingsToWrite.sensors[2].macAddress));

//strcpy(settingsToWrite.sensors[0].name,"	Tacx Flux");
//memcpy(settingsToWrite.sensors[0].macAddress, macFlux, sizeof(settingsToWrite.sensors[0].macAddress));


  //saveToFlash((uint8_t*) &settingsToWrite, sizeof(settingsToWrite));


/* USER CODE END APP_BLE_Init_2 */
  return;
}

void Trigger_Scan_Request( void ){
	UTIL_SEQ_SetTask(1 << CFG_TASK_START_SCAN_ID, CFG_SCH_PRIO_1);
}

void Trigger_Connection_Request( int indexInFlash,int indexInScannedDevices,Pairing_request_status status, uint16_t connhandle){ //alexis a modifier
	switch(indexInFlash){
		case 0:
			if(status == CONNECTING){
				UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_1_ID, CFG_SCH_PRIO_0);
			}
			else{
				aci_gap_terminate(connhandle, 0x05);
				usedDeviceInformations[0].state = APP_BLE_IDLE;
				scannedDevicesPackage.scannedDevicesList[indexInScannedDevices].pairingStatus = usedDeviceInformations[0].state;
			}

			break;
		case 1:
			if(status == CONNECTING){
				UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_2_ID, CFG_SCH_PRIO_0);
			}
			else{
				aci_gap_terminate(connhandle, 0x05);
				usedDeviceInformations[1].state = APP_BLE_IDLE;
				scannedDevicesPackage.scannedDevicesList[indexInScannedDevices].pairingStatus = usedDeviceInformations[1].state;
			}

			break;
		case 2:
			if(status == CONNECTING){
				UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_3_ID, CFG_SCH_PRIO_0);
			}
			else{
				aci_gap_terminate(connhandle, 0x05);
				usedDeviceInformations[2].state = APP_BLE_IDLE;
				scannedDevicesPackage.scannedDevicesList[indexInScannedDevices].pairingStatus = usedDeviceInformations[2].state;
			}

			break;
		case 3:
			if(status == CONNECTING){
				UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_4_ID, CFG_SCH_PRIO_0);
			}
			else{
				aci_gap_terminate(connhandle, 0x05);
				usedDeviceInformations[3].state = APP_BLE_IDLE;
				scannedDevicesPackage.scannedDevicesList[indexInScannedDevices].pairingStatus = usedDeviceInformations[3].state;
			}

			break;
		default:
			break;
	}
}

uint16_t Update_UsedDeviceInformations_structure( void )
{
    settings_t readSettings;
    readFlash((uint8_t*)&readSettings);

    int isFlashEmpty = 0;
    int isUsedDeviceEmpty = 0;
    uint16_t connhandle = 0;

    //used to add devices in usedDeviceStructure

    for(int i = 0; i < sizeof(readSettings.sensors) / sizeof(readSettings.sensors[0]); i++){
    	for(int k = 0; k < scannedDevicesPackage.numberOfScannedDevices; k++){
    		if(strcmp(scannedDevicesPackage.scannedDevicesList[k].deviceName, readSettings.sensors[i].name) == 0 ||
    				memcmp(scannedDevicesPackage.scannedDevicesList[k].macAddress, readSettings.sensors[i].macAddress, sizeof(readSettings.sensors[i].macAddress)) == 0){

    			isFlashEmpty = 0; //reset between iterations

        		for(int idx = 0; idx < sizeof(readSettings.sensors[i].name);idx++){
        			isFlashEmpty |= readSettings.sensors[i].name[idx];
        		}
        		if(isFlashEmpty != 0){ //flash is not empty

        			isUsedDeviceEmpty = 0; //reset between iterations

            		for(int idx = 0; idx < sizeof(usedDeviceInformations[i].name);idx++){
            			isUsedDeviceEmpty |= usedDeviceInformations[i].name[idx];
            		}
            		if(isUsedDeviceEmpty == 0){ //used device is empty - newly added
            			strcpy(usedDeviceInformations[i].name, readSettings.sensors[i].name);
            			memcpy(usedDeviceInformations[i].macAddress, readSettings.sensors[i].macAddress, sizeof(usedDeviceInformations[i].macAddress));
            			usedDeviceInformations[i].isNotEmpty = true;
            			usedDeviceInformations[i].connHandle = 0xFFFF;  // a changer
            		}
        		}
    		}
    	}
    }
    //used to remove devices in usedDeviceStructure

    for(int i = 0; i < sizeof(readSettings.sensors) / sizeof(readSettings.sensors[0]); i++){
    	for(int k = 0; k < sizeof(usedDeviceInformations)/ sizeof(usedDeviceInformations[0]); k++){
    		if(memcmp(readSettings.sensors[i].macAddress, usedDeviceInformations[k].macAddress, sizeof(usedDeviceInformations[k].macAddress)) != 0 && i == k){
    			connhandle = usedDeviceInformations[k].connHandle;
    			memset(&usedDeviceInformations[k],0,sizeof(usedDeviceInformations[k]));
    			break;
    		}
    	}
    }

    return connhandle;

}

SVCCTL_UserEvtFlowStatus_t SVCCTL_App_Notification(void *pckt)
{
  hci_event_pckt *event_pckt;
  evt_le_meta_event *meta_evt;
  hci_le_connection_complete_event_rp0 * connection_complete_event;
  evt_blue_aci *blue_evt;
  hci_le_advertising_report_event_rp0 * le_advertising_event;
  event_pckt = (hci_event_pckt*) ((hci_uart_pckt *) pckt)->data;
  uint8_t result;
  uint8_t role, event_type, event_data_size;
  int k = 0;
  uint8_t *adv_report_data;
  uint8_t adtype, adlength;
  hci_disconnection_complete_event_rp0 *cc = (void *) event_pckt->data;

	//printf("event_pckt: %x\n\r",event_pckt->evt);
  switch (event_pckt->evt)
  {
    /* USER CODE BEGIN evt */

    /* USER CODE END evt */
    case EVT_VENDOR:
    {
      handleNotification.P2P_Evt_Opcode = P2P_SERVER1_DISCON_HANDLE_EVT;
      blue_evt = (evt_blue_aci*) event_pckt->data;
      /* USER CODE BEGIN EVT_VENDOR */
      //printf("ecode: %x\n\r",blue_evt->ecode);
      /* USER CODE END EVT_VENDOR */

      switch (blue_evt->ecode)
      {
      /* USER CODE BEGIN ecode */

      /* USER CODE END ecode */
        case EVT_BLUE_GAP_PROCEDURE_COMPLETE:
        {
          /* USER CODE BEGIN EVT_BLUE_GAP_PROCEDURE_COMPLETE */

          /* USER CODE END EVT_BLUE_GAP_PROCEDURE_COMPLETE */
          aci_gap_proc_complete_event_rp0 *gap_evt_proc_complete = (void*) blue_evt->data;
          /* CHECK GAP GENERAL DISCOVERY PROCEDURE COMPLETED & SUCCEED */
          if (gap_evt_proc_complete->Procedure_Code == GAP_GENERAL_DISCOVERY_PROC
              && gap_evt_proc_complete->Status == 0x00)
          {
            /* USER CODE BEGIN GAP_GENERAL_DISCOVERY_PROC */
            //BSP_LED_Off(LED_BLUE);

            /* USER CODE END GAP_GENERAL_DISCOVERY_PROC */

            Update_UsedDeviceInformations_structure();

            APP_DBG_MSG("-- GAP GENERAL DISCOVERY PROCEDURE_COMPLETED\n");
            /*if a device found, connect to it, device 1 being chosen first if both found*/
            if (usedDeviceInformations[0].isNotEmpty
                && usedDeviceInformations[0].state != APP_BLE_CONNECTED_CLIENT)
            {
              UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_1_ID, CFG_SCH_PRIO_0);
            }
#if (CFG_P2P_DEMO_MULTI != 0)
          /* USER CODE BEGIN EVT_BLUE_GAP_PROCEDURE_COMPLETE_Multi */
            else if (usedDeviceInformations[1].isNotEmpty
            		&& usedDeviceInformations[1].state != APP_BLE_CONNECTED_CLIENT)
            {
              UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_2_ID, CFG_SCH_PRIO_0);
            }
            else if (usedDeviceInformations[2].isNotEmpty
                && usedDeviceInformations[2].state != APP_BLE_CONNECTED_CLIENT)
            {
              UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_3_ID, CFG_SCH_PRIO_0);
            }
            else if (usedDeviceInformations[3].isNotEmpty
                && usedDeviceInformations[3].state != APP_BLE_CONNECTED_CLIENT)
            {
              UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_4_ID, CFG_SCH_PRIO_0);
            }
          /* USER CODE END EVT_BLUE_GAP_PROCEDURE_COMPLETE_Multi */
#endif

          }

        }
        break; /* EVT_BLUE_GAP_PAIRING_CMPLT */

        case EVT_BLUE_L2CAP_CONNECTION_UPDATE_REQ:
        {
         /* USER CODE BEGIN EVT_BLUE_L2CAP_CONNECTION_UPDATE_REQ */

          /* USER CODE END EVT_BLUE_L2CAP_CONNECTION_UPDATE_REQ */
          aci_l2cap_connection_update_req_event_rp0 *pr = (aci_l2cap_connection_update_req_event_rp0 *) blue_evt->data;
          APP_BLE_p2p_Conn_Update_req.Connection_Handle = pr->Connection_Handle;
          APP_BLE_p2p_Conn_Update_req.Identifier = pr->Identifier;
          APP_BLE_p2p_Conn_Update_req.L2CAP_Length = pr->L2CAP_Length;
          APP_BLE_p2p_Conn_Update_req.Interval_Min = pr->Interval_Min;
          APP_BLE_p2p_Conn_Update_req.Interval_Max = pr->Interval_Max;
          APP_BLE_p2p_Conn_Update_req.Slave_Latency = pr->Slave_Latency;
          APP_BLE_p2p_Conn_Update_req.Timeout_Multiplier = pr->Timeout_Multiplier;

          /*printf("\n interval min :%d",pr->Interval_Min);
          printf("\n interval max :%d",pr->Interval_Max);
          printf("\n slave latency :%d",pr->Slave_Latency);
          printf("\n timeout_multi :%d",pr->Timeout_Multiplier);*/

          result = aci_l2cap_connection_parameter_update_resp(APP_BLE_p2p_Conn_Update_req.Connection_Handle,
                                                              APP_BLE_p2p_Conn_Update_req.Interval_Min,
                                                              APP_BLE_p2p_Conn_Update_req.Interval_Max,
                                                              APP_BLE_p2p_Conn_Update_req.Slave_Latency,
                                                              APP_BLE_p2p_Conn_Update_req.Timeout_Multiplier,
                                                              CONN_L(10),
                                                              CONN_L(10),
                                                              APP_BLE_p2p_Conn_Update_req.Identifier,
                                                              0x01);
          APP_DBG_MSG("\r\n\r** NO UPDATE \n");
          if(result != BLE_STATUS_SUCCESS) {
              /* USER CODE BEGIN BLE_STATUS_SUCCESS */
              BSP_LED_On(LED_RED);
              /* USER CODE END BLE_STATUS_SUCCESS */
          }

        }

        break;

        default:
          /* USER CODE BEGIN ecode_default */

          /* USER CODE END ecode_default */
          break;
      }
    }
    break; /* EVT_VENDOR */

    case EVT_DISCONN_COMPLETE:

      /* USER CODE BEGIN EVT_DISCONN_COMPLETE */


      /* USER CODE END EVT_DISCONN_COMPLETE */

    	if(cc->Reason != 0x05){ //user terminated connection
    	      if (cc->Connection_Handle == BleApplicationContext.connectionHandleEndDevice1)
    	      {

    	        APP_DBG_MSG("\r\n\r** DISCONNECTION EVENT OF END DEVICE 1 \n");
    	        usedDeviceInformations[0].state = APP_BLE_IDLE;
    	        //scannedDevicesPackage.scannedDevicesList[index].pairingStatus =  BleApplicationContext.EndDevice_Connection_Status[0];
    	        usedDeviceInformations[0].connHandle = 0xFFFF;
    	        handleNotification.P2P_Evt_Opcode = P2P_SERVER1_DISCON_HANDLE_EVT;
    	        handleNotification.ConnectionHandle = connection_handle;
    	        Evt_Notification(&handleNotification);
    	      }

    	      if (cc->Connection_Handle == BleApplicationContext.connectionHandleCentral)
    	      {
    	        APP_DBG_MSG("\r\n\r** DISCONNECTION EVENT OF SMART PHONE \n");
    	        BleApplicationContext.connectionHandleCentral = APP_BLE_IDLE;
    	        handleNotification.P2P_Evt_Opcode = SMART_PHONE1_DISCON_HANDLE_EVT;
    	        handleNotification.ConnectionHandle = 0xFFFF;
    	        Evt_Notification(&handleNotification);
    	      }

    	#if (CFG_P2P_DEMO_MULTI != 0)
    	      /* USER CODE BEGIN EVT_DISCONN_COMPLETE_Multi */
    	      if (cc->Connection_Handle == BleApplicationContext.connectionHandleEndDevice2)
    	      {

    	        APP_DBG_MSG("\r\n\r** DISCONNECTION EVENT OF END DEVICE 2 \n");
    	        usedDeviceInformations[1].state = APP_BLE_IDLE;
    	        //scannedDevicesPackage.scannedDevicesList[index].pairingStatus =  BleApplicationContext.EndDevice_Connection_Status[1];
    	        usedDeviceInformations[1].connHandle = 0xFFFF;
    	        handleNotification.P2P_Evt_Opcode = P2P_SERVER2_DISCON_HANDLE_EVT;
    	        handleNotification.ConnectionHandle = connection_handle;
    	        Evt_Notification(&handleNotification);
    	      }
    	      if (cc->Connection_Handle == BleApplicationContext.connectionHandleEndDevice3)
    	      {

    	        APP_DBG_MSG("\r\n\r** DISCONNECTION EVENT OF END DEVICE 3 \n");
    	        usedDeviceInformations[2].state = APP_BLE_IDLE;
    	        //scannedDevicesPackage.scannedDevicesList[index].pairingStatus =  BleApplicationContext.EndDevice_Connection_Status[2];
    	        usedDeviceInformations[2].connHandle = 0xFFFF;
    	        handleNotification.P2P_Evt_Opcode = P2P_SERVER3_DISCON_HANDLE_EVT;
    	        handleNotification.ConnectionHandle = connection_handle;
    	        Evt_Notification(&handleNotification);
    	      }
    	      if (cc->Connection_Handle == BleApplicationContext.connectionHandleEndDevice4)
    	      {

    	        APP_DBG_MSG("\r\n\r** DISCONNECTION EVENT OF END DEVICE 4 \n");
    	        usedDeviceInformations[3].state = APP_BLE_IDLE;
    	        //scannedDevicesPackage.scannedDevicesList[index].pairingStatus =  BleApplicationContext.EndDevice_Connection_Status[3];
    	        usedDeviceInformations[3].connHandle = 0xFFFF;
    	        handleNotification.P2P_Evt_Opcode = P2P_SERVER4_DISCON_HANDLE_EVT;
    	        handleNotification.ConnectionHandle = connection_handle;
    	        Evt_Notification(&handleNotification);
    	      }


    	      /* USER CODE END EVT_DISCONN_COMPLETE_Multi */
    	#endif
    	}


      break; /* EVT_DISCONN_COMPLETE */

    case EVT_LE_META_EVENT:

      /* USER CODE BEGIN EVT_LE_META_EVENT */

      /* USER CODE END EVT_LE_META_EVENT */
      meta_evt = (evt_le_meta_event*) event_pckt->data;

      switch (meta_evt->subevent)
      {
      /* USER CODE BEGIN subevent */

      /* USER CODE END subevent */
        case EVT_LE_CONN_COMPLETE:
          /* USER CODE BEGIN EVT_LE_CONN_COMPLETE */



          /* USER CODE END EVT_LE_CONN_COMPLETE */
          /**
           * The connection is done, there is no need anymore to schedule the LP ADV
           */
          connection_complete_event = (hci_le_connection_complete_event_rp0 *) meta_evt->data;

          connection_handle = connection_complete_event->Connection_Handle;
          role = connection_complete_event->Role;
          if (role == 0x00)
          { /* ROLE MASTER */

            uint8_t dev1 = 1
#if (CFG_P2P_DEMO_MULTI != 0)
          /* USER CODE BEGIN EVT_LE_CONN_COMPLETE_Multi */
           , dev2 = 1,dev3 = 1,dev4 = 1
          /* USER CODE END EVT_LE_CONN_COMPLETE_Multi */
#endif
                ;


            for (int i = 0; i < 6; i++)
            {
              dev1 &= (usedDeviceInformations[0].macAddress[i] == connection_complete_event->Peer_Address[i]);
#if (CFG_P2P_DEMO_MULTI != 0)
          /* USER CODE BEGIN EVT_LE_CONN_COMPLETE_Multi_2 */
              dev2 &= (usedDeviceInformations[1].macAddress[i] == connection_complete_event->Peer_Address[i]);
              dev3 &= (usedDeviceInformations[2].macAddress[i] == connection_complete_event->Peer_Address[i]);
              dev4 &= (usedDeviceInformations[3].macAddress[i] == connection_complete_event->Peer_Address[i]);
          /* USER CODE END EVT_LE_CONN_COMPLETE_Multi_2 */
#endif
            }

            if (dev1 == 1)
            {
              /* Inform Application it is End Device 1 */
              int index0 = getCorrespondingIndex(usedDeviceInformations[0].macAddress);

              APP_DBG_MSG("-- CONNECTION SUCCESS WITH END DEVICE 1\n");
              usedDeviceInformations[0].state = APP_BLE_CONNECTED_CLIENT;
              usedDeviceInformations[0].connHandle = connection_handle;
              scannedDevicesPackage.scannedDevicesList[index0].pairingStatus =  usedDeviceInformations[0].state;
              BleApplicationContext.connectionHandleEndDevice1 = connection_handle;
              BleApplicationContext.BleApplicationContext_legacy.connectionHandle[0] = connection_handle;
              handleNotification.P2P_Evt_Opcode = P2P_SERVER1_CONN_HANDLE_EVT;
              handleNotification.ConnectionHandle = connection_handle;
              Evt_Notification(&handleNotification);
              result = aci_gatt_disc_all_primary_services(BleApplicationContext.connectionHandleEndDevice1);
              if (result == BLE_STATUS_SUCCESS)
              {
                APP_DBG_MSG("\r\n\r** GATT SERVICES & CHARACTERISTICS DISCOVERY  \n");
                APP_DBG_MSG("* GATT :  Start Searching Primary Services \r\n\r");
              }
              else
              {
                APP_DBG_MSG("BLE_CTRL_App_Notification(), All services discovery Failed \r\n\r");
              }
#if (CFG_P2P_DEMO_MULTI != 0)
          /* USER CODE BEGIN EVT_LE_CONN_COMPLETE_Multi_3 */
          /* Now try to connect to device 2 */
              if ((usedDeviceInformations[1].state != APP_BLE_CONNECTED_CLIENT)
                  && (usedDeviceInformations[1].isNotEmpty))
              {
                UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_2_ID, CFG_SCH_PRIO_0);
              }
          /* USER CODE END EVT_LE_CONN_COMPLETE_Multi_3 */
#endif

            }
#if (CFG_P2P_DEMO_MULTI != 0)
          /* USER CODE BEGIN EVT_LE_CONN_COMPLETE_Multi_4 */
          if (dev2 == 1)
            {
              /* Inform Application it is End Device 2 */
        	  int index1 = getCorrespondingIndex(usedDeviceInformations[1].macAddress);

              APP_DBG_MSG("-- CONNECTION SUCCESS WITH END DEVICE 2\n");
              usedDeviceInformations[1].state = APP_BLE_CONNECTED_CLIENT;
              usedDeviceInformations[1].connHandle = connection_handle;
              scannedDevicesPackage.scannedDevicesList[index1].pairingStatus =  usedDeviceInformations[1].state;
              BleApplicationContext.connectionHandleEndDevice2 = connection_handle;
              BleApplicationContext.BleApplicationContext_legacy.connectionHandle[1] = connection_handle;
              handleNotification.P2P_Evt_Opcode = P2P_SERVER2_CONN_HANDLE_EVT;
              handleNotification.ConnectionHandle = connection_handle;
              Evt_Notification(&handleNotification);
              result = aci_gatt_disc_all_primary_services(BleApplicationContext.connectionHandleEndDevice2);
              if (result == BLE_STATUS_SUCCESS)
              {
                APP_DBG_MSG("* GATT : Start Searching Primary Services \r\n\r");
              }
              else
              {
                APP_DBG_MSG("BLE_CTRL_App_Notification(), All services discovery Failed \r\n\r");
              }
              /* Now try to connect to device 1 */
              if ((usedDeviceInformations[2].state != APP_BLE_CONNECTED_CLIENT)
                  && (usedDeviceInformations[2].isNotEmpty))
              {

                UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_3_ID, CFG_SCH_PRIO_0);
              }
            }
            if (dev3 == 1)
            {
              /* Inform Application it is End Device 3 */

              int index2 = getCorrespondingIndex(usedDeviceInformations[2].macAddress);

              APP_DBG_MSG("-- CONNECTION SUCCESS WITH END DEVICE 3\n");

              usedDeviceInformations[2].state = APP_BLE_CONNECTED_CLIENT;
              usedDeviceInformations[2].connHandle = connection_handle;
              scannedDevicesPackage.scannedDevicesList[index2].pairingStatus =  usedDeviceInformations[2].state;
              BleApplicationContext.connectionHandleEndDevice3 = connection_handle;
              BleApplicationContext.BleApplicationContext_legacy.connectionHandle[2] = connection_handle;
              handleNotification.P2P_Evt_Opcode = P2P_SERVER3_CONN_HANDLE_EVT;
              handleNotification.ConnectionHandle = connection_handle;
              Evt_Notification(&handleNotification);
              result = aci_gatt_disc_all_primary_services(BleApplicationContext.connectionHandleEndDevice3);
              if (result == BLE_STATUS_SUCCESS)
              {
                APP_DBG_MSG("* GATT : Start Searching Primary Services \r\n\r");
              }
              else
              {
                APP_DBG_MSG("BLE_CTRL_App_Notification(), All services discovery Failed \r\n\r");
              }
              /* Now try to connect to device 4 */
              if ((usedDeviceInformations[3].state != APP_BLE_CONNECTED_CLIENT)
                  && (usedDeviceInformations[3].isNotEmpty))
              {

                UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_4_ID, CFG_SCH_PRIO_0);
              }
            }
            if (dev4 == 1)
            {
              /* Inform Application it is End Device 4 */

              int index3 = getCorrespondingIndex(usedDeviceInformations[3].macAddress);

              APP_DBG_MSG("-- CONNECTION SUCCESS WITH END DEVICE 4\n");

              usedDeviceInformations[3].state = APP_BLE_CONNECTED_CLIENT;
              usedDeviceInformations[3].connHandle = connection_handle;
              scannedDevicesPackage.scannedDevicesList[index3].pairingStatus =  usedDeviceInformations[3].state;
              BleApplicationContext.connectionHandleEndDevice4 = connection_handle;
              BleApplicationContext.BleApplicationContext_legacy.connectionHandle[3] = connection_handle;
              handleNotification.P2P_Evt_Opcode = P2P_SERVER4_CONN_HANDLE_EVT;
              handleNotification.ConnectionHandle = connection_handle;
              Evt_Notification(&handleNotification);
              result = aci_gatt_disc_all_primary_services(BleApplicationContext.connectionHandleEndDevice4);
              if (result == BLE_STATUS_SUCCESS)
              {
                APP_DBG_MSG("* GATT : Start Searching Primary Services \r\n\r");
              }
              else
              {
                APP_DBG_MSG("BLE_CTRL_App_Notification(), All services discovery Failed \r\n\r");
              }
              /* Now try to connect to device 3 */
              if ((usedDeviceInformations[2].state != APP_BLE_CONNECTED_CLIENT)
                  && (usedDeviceInformations[2].isNotEmpty))
              {

                UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_3_ID, CFG_SCH_PRIO_0);
              }
            }

          /* USER CODE END EVT_LE_CONN_COMPLETE_Multi_4 */
#endif
          }

          else
          {
            APP_DBG_MSG("-- CONNECTION SUCCESS WITH SMART PHONE\n");
            BleApplicationContext.connectionHandleCentral = connection_handle;
            handleNotification.P2P_Evt_Opcode = SMART_PHONE1_CONN_HANDLE_EVT;
            handleNotification.ConnectionHandle = connection_handle;
            Evt_Notification(&handleNotification);
          }

          break; /* HCI_EVT_LE_CONN_COMPLETE */

        case EVT_LE_ADVERTISING_REPORT:

          /* USER CODE BEGIN EVT_LE_ADVERTISING_REPORT */

          /* USER CODE END EVT_LE_ADVERTISING_REPORT */
          le_advertising_event = (hci_le_advertising_report_event_rp0 *) meta_evt->data;

          event_type = le_advertising_event->Advertising_Report[0].Event_Type;

          event_data_size = le_advertising_event->Advertising_Report[0].Length_Data;

          /* WARNING: be careful when decoding advertising report as its raw format cannot be mapped on a C structure.
          The data and RSSI values could not be directly decoded from the RAM using the data and RSSI field from hci_le_advertising_report_event_rp0 structure.
          Instead they must be read by using offsets (please refer to BLE specification).
          RSSI = *(uint8_t*) (adv_report_data + le_advertising_event->Advertising_Report[0].Length_Data);
          */
          adv_report_data = (uint8_t*)(&le_advertising_event->Advertising_Report[0].Length_Data) + 1;
          k = 0;

          /* search AD TYPE 0x09 (Complete Local Name) */
          /* search AD Type 0x02 (16 bits UUIDS) */
          if (event_type == ADV_IND ||event_type==SCAN_RSP)
          {

            /* ISOLATION OF BD ADDRESS AND LOCAL NAME */

            while(k < event_data_size)
            {
              adlength = adv_report_data[k];
              adtype = adv_report_data[k + 1];
              char current_device_name[MAX_DEVICE_NAME_LENGHT];
              memset(current_device_name, 0, MAX_DEVICE_NAME_LENGHT);
              bool isStringFound = false;

              switch (adtype)
              {
                case 0x01: /* now get flags */
                /* USER CODE BEGIN get_flags */

                /* USER CODE END get_flags */
                break;
                case 0x09: /* now get local name */
                /* USER CODE BEGIN get_local_name */

                	for (int i=0; i<adlength;i++){
                		current_device_name[i] = adv_report_data[k+i+1];
					}

                	for(int j=0;j<scannedDevicesPackage.numberOfScannedDevices;j++){
                		if (strcmp(current_device_name, scannedDevicesPackage.scannedDevicesList[j].deviceName) == 0){
                			isStringFound = true;
                			break;
                		}
                	}
					if (isStringFound == false){
						scannedDevicesPackage.iterator = scannedDevicesPackage.numberOfScannedDevices;
						strcpy(scannedDevicesPackage.scannedDevicesList[scannedDevicesPackage.iterator].deviceName, current_device_name);
						scannedDevicesPackage.scannedDevicesList[scannedDevicesPackage.iterator].pairingStatus = DISCONNECT;
						scannedDevicesPackage.scannedDevicesList[scannedDevicesPackage.iterator].position = scannedDevicesPackage.iterator;
						scannedDevicesPackage.scannedDevicesList[scannedDevicesPackage.iterator].macAddress[0] = le_advertising_event->Advertising_Report[0].Address[0];
						scannedDevicesPackage.scannedDevicesList[scannedDevicesPackage.iterator].macAddress[1] = le_advertising_event->Advertising_Report[0].Address[1];
						scannedDevicesPackage.scannedDevicesList[scannedDevicesPackage.iterator].macAddress[2] = le_advertising_event->Advertising_Report[0].Address[2];
						scannedDevicesPackage.scannedDevicesList[scannedDevicesPackage.iterator].macAddress[3] = le_advertising_event->Advertising_Report[0].Address[3];
						scannedDevicesPackage.scannedDevicesList[scannedDevicesPackage.iterator].macAddress[4] = le_advertising_event->Advertising_Report[0].Address[4];
						scannedDevicesPackage.scannedDevicesList[scannedDevicesPackage.iterator].macAddress[5] = le_advertising_event->Advertising_Report[0].Address[5];

						printf("%s", scannedDevicesPackage.scannedDevicesList[scannedDevicesPackage.iterator].deviceName);
						printf(" //////// %d /////////// new\n", scannedDevicesPackage.iterator);
						scannedDevicesPackage.iterator ++;
						scannedDevicesPackage.numberOfScannedDevices ++;

				        EDS_STM_Update_Char(0x0001,
				                (uint8_t *)&scannedDevicesPackage.numberOfScannedDevices);

					}

                /* USER CODE END get_local_name */
                  break;

                case 0x07: //manufacture name

                break;

                case 0x02: /* now get UID */
                /* USER CODE BEGIN get_UID */

                /* USER CODE END get_UID */
                  break;
                case 0x0A: /* Tx power level */
                /* USER CODE BEGIN Tx_power_level */

                /* USER CODE END Tx_power_level */
                  break;
                case 0xFF: /* Manufacturer Specific */
                /* USER CODE BEGIN Manufactureur_Specific */
                	break;
                case 0x16:
                  /* USER CODE BEGIN AD_TYPE_SERVICE_DATA */

                  /* USER CODE END AD_TYPE_SERVICE_DATA */
                  break;
                default:
                  /* USER CODE BEGIN adtype_default */

                  /* USER CODE END adtype_default */
                  break;
              }
              k += adlength + 1;
            }

          }

          break;

      }

      break; /* HCI_EVT_LE_META_EVENT */
    }
  return (SVCCTL_UserEvtFlowEnable);
}

/*************************************************************
 *
 * PUBLIC FUNCTIONS
 *
 *************************************************************/

APP_BLE_ConnStatus_t APP_BLE_Get_Client_Connection_Status( uint16_t Connection_Handle )
{
  /* USER CODE BEGIN APP_BLE_Get_Client_Connection_Status_1 */

  /* USER CODE END APP_BLE_Get_Client_Connection_Status_1 */
  APP_BLE_ConnStatus_t return_value;

  if (usedDeviceInformations[0].connHandle == Connection_Handle)
  {
    return_value = usedDeviceInformations[0].state;
  }
#if (CFG_P2P_DEMO_MULTI != 0)
/* USER CODE BEGIN APP_BLE_Get_Client_Connection_Status_Multi */
  else if (usedDeviceInformations[1].connHandle == Connection_Handle)
  {
    return_value = usedDeviceInformations[1].state;
  }
  else if (usedDeviceInformations[2].connHandle == Connection_Handle)
  {
    return_value = usedDeviceInformations[2].state;
  }
  else if (usedDeviceInformations[3].connHandle == Connection_Handle)
  {
    return_value = usedDeviceInformations[3].state;
  }
/* USER CODE END APP_BLE_Get_Client_Connection_Status_Multi */
#endif
  else
  {
    return_value = APP_BLE_IDLE;
  }
  /* USER CODE BEGIN APP_BLE_Get_Client_Connection_Status_2 */

  /* USER CODE END APP_BLE_Get_Client_Connection_Status_2 */
  return (return_value);
}


/* USER CODE BEGIN FD */
void APP_BLE_Key_Button1_Action(void)
{
  UTIL_SEQ_SetTask(1 << CFG_TASK_START_SCAN_ID, CFG_SCH_PRIO_0);
}

void APP_BLE_Key_Button2_Action(void)
{
}

void APP_BLE_Key_Button3_Action(void)
{
}

/* USER CODE END FD */
/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
static void Ble_Tl_Init( void )
{
  HCI_TL_HciInitConf_t Hci_Tl_Init_Conf;

  Hci_Tl_Init_Conf.p_cmdbuffer = (uint8_t*)&BleCmdBuffer;
  Hci_Tl_Init_Conf.StatusNotCallBack = BLE_StatusNot;
  hci_init(BLE_UserEvtRx, (void*) &Hci_Tl_Init_Conf);

  return;
}

static void Ble_Hci_Gap_Gatt_Init(void){

  uint8_t role;
  uint8_t index;
  uint16_t gap_service_handle, gap_dev_name_char_handle, gap_appearance_char_handle;
  const uint8_t *bd_addr;
  uint32_t srd_bd_addr[2];
  uint16_t appearance[1] = { BLE_CFG_UNKNOWN_APPEARANCE };

  /**
   * Initialize HCI layer
   */
  /*HCI Reset to synchronise BLE Stack*/
  hci_reset();

  /**
   * Write the BD Address
   */

  bd_addr = BleGetBdAddress();
  aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
                            CONFIG_DATA_PUBADDR_LEN,
                            (uint8_t*) bd_addr);
  /* BLE MAC in ADV Packet */
  manuf_data[ sizeof(manuf_data)-6] = bd_addr[5];
  manuf_data[ sizeof(manuf_data)-5] = bd_addr[4];
  manuf_data[ sizeof(manuf_data)-4] = bd_addr[3];
  manuf_data[ sizeof(manuf_data)-3] = bd_addr[2];
  manuf_data[ sizeof(manuf_data)-2] = bd_addr[1];
  manuf_data[ sizeof(manuf_data)-1] = bd_addr[0];

  /**
   * Write Identity root key used to derive LTK and CSRK
   */
    aci_hal_write_config_data(CONFIG_DATA_IR_OFFSET,
    CONFIG_DATA_IR_LEN,
                            (uint8_t*) BLE_CFG_IR_VALUE);

   /**
   * Write Encryption root key used to derive LTK and CSRK
   */
    aci_hal_write_config_data(CONFIG_DATA_ER_OFFSET,
    CONFIG_DATA_ER_LEN,
                            (uint8_t*) BLE_CFG_ER_VALUE);

  /**
   * Static random Address
   * The two upper bits shall be set to 1
   * The lowest 32bits is read from the UDN to differentiate between devices
   * The RNG may be used to provide a random number on each power on
   */
  srd_bd_addr[1] =  0x0000ED6E;
  srd_bd_addr[0] =  LL_FLASH_GetUDN( );
  aci_hal_write_config_data( CONFIG_DATA_RANDOM_ADDRESS_OFFSET, CONFIG_DATA_RANDOM_ADDRESS_LEN, (uint8_t*)srd_bd_addr );

  /**
   * Write Identity root key used to derive LTK and CSRK
   */
    aci_hal_write_config_data( CONFIG_DATA_IR_OFFSET, CONFIG_DATA_IR_LEN, (uint8_t*)BLE_CFG_IR_VALUE );

   /**
   * Write Encryption root key used to derive LTK and CSRK
   */
    aci_hal_write_config_data( CONFIG_DATA_ER_OFFSET, CONFIG_DATA_ER_LEN, (uint8_t*)BLE_CFG_ER_VALUE );

  /**
   * Set TX Power to 0dBm.
   */
  aci_hal_set_tx_power_level(1, CFG_TX_POWER);

  /**
   * Initialize GATT interface
   */
  aci_gatt_init();

  /**
   * Initialize GAP interface
   */
  role = 0;

#if (BLE_CFG_PERIPHERAL == 1)
  role |= GAP_PERIPHERAL_ROLE;
#endif

#if (BLE_CFG_CENTRAL == 1)
  role |= GAP_CENTRAL_ROLE;
#endif

  if (role > 0)
  {
    const char *name = "MissLP";

    aci_gap_init(role, 0,
                 APPBLE_GAP_DEVICE_NAME_LENGTH,
                 &gap_service_handle, &gap_dev_name_char_handle, &gap_appearance_char_handle);

    if (aci_gatt_update_char_value(gap_service_handle, gap_dev_name_char_handle, 0, strlen(name), (uint8_t *) name))
    {
      BLE_DBG_SVCCTL_MSG("Device Name aci_gatt_update_char_value failed.\n");
    }
  }

  if(aci_gatt_update_char_value(gap_service_handle,
                                gap_appearance_char_handle,
                                0,
                                2,
                                (uint8_t *)&appearance))
  {
    BLE_DBG_SVCCTL_MSG("Appearance aci_gatt_update_char_value failed.\n");
  }

  /**
   * Initialize IO capability
   */
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.ioCapability = CFG_IO_CAPABILITY;
  aci_gap_set_io_capability(BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.ioCapability);

  /**
   * Initialize authentication
   */
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.mitm_mode = CFG_MITM_PROTECTION;
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.OOB_Data_Present = 0;
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMin = 8;
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMax = 16;
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.Use_Fixed_Pin = 1;
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.Fixed_Pin = 111111;
  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode = 1;
  for (index = 0; index < 16; index++)
  {
    BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.OOB_Data[index] = (uint8_t) index;
  }

  aci_gap_set_authentication_requirement(BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode,
                                         BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.mitm_mode,
                                         1,
                                         0,
                                         BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMin,
                                         BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMax,
                                         BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.Use_Fixed_Pin,
                                         BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.Fixed_Pin,
                                         0
  );

  /**
   * Initialize whitelist
   */
   if (BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode)
   {
     aci_gap_configure_whitelist();
   }

}

/**
 * @brief  Scan Request
 * @param  None
 * @retval None
 */
static void Scan_Request( void )
{
  /* USER CODE BEGIN Scan_Request_1 */

  /* USER CODE END Scan_Request_1 */
  tBleStatus result;

  if (BleApplicationContext.EndDevice_Connection_Status[0] != APP_BLE_CONNECTED_CLIENT

#if (CFG_P2P_DEMO_MULTI != 0)
      || BleApplicationContext.EndDevice_Connection_Status[1] != APP_BLE_CONNECTED_CLIENT || BleApplicationContext.EndDevice_Connection_Status[2] != APP_BLE_CONNECTED_CLIENT
      || BleApplicationContext.EndDevice_Connection_Status[3] != APP_BLE_CONNECTED_CLIENT|| BleApplicationContext.EndDevice_Connection_Status[4] != APP_BLE_CONNECTED_CLIENT
#endif
  )
  {
    /* USER CODE BEGIN APP_BLE_CONNECTED */
    //BSP_LED_On(LED_BLUE);
    /* USER CODE END APP_BLE_CONNECTED */
    result = aci_gap_start_general_discovery_proc(SCAN_P, SCAN_L, PUBLIC_ADDR, 1);
    if (result == BLE_STATUS_SUCCESS)
    {
    /* USER CODE BEGIN BLE_SCAN_SUCCESS */
    //device_list_index = 0;

    /* USER CODE END BLE_SCAN_SUCCESS */
      APP_DBG_MSG(" \r\n\r** START GENERAL DISCOVERY (SCAN) **  \r\n\r");
    }
    else
    {
    /* USER CODE BEGIN BLE_SCAN_FAILED */
      BSP_LED_On(LED_RED);
    /* USER CODE END BLE_SCAN_FAILED */
      APP_DBG_MSG("-- BLE_App_Start_Limited_Disc_Req, Failed \r\n\r");
    }
  }
  /* USER CODE BEGIN Scan_Request_2 */

  /* USER CODE END Scan_Request_2 */
  return;
}

/**
 * @brief  Advertising Enable
 * @param  None
 * @retval None
 */
static void Adv_Request( void )
{
  /* USER CODE BEGIN Connect_Request_1 */

  /* USER CODE END Connect_Request_1 */
  if (BleApplicationContext.SmartPhone_Connection_Status != APP_BLE_CONNECTED_SERVER)
  {
    tBleStatus result = 0x00;
    /*Start Advertising*/
    result = aci_gap_set_discoverable(ADV_IND,
                                      LEDBUTTON_CONN_ADV_INTERVAL_MIN,
                                      LEDBUTTON_CONN_ADV_INTERVAL_MAX,
                                      PUBLIC_ADDR,
                                      NO_WHITE_LIST_USE, /* use white list */
                                      sizeof(local_name),
                                      (uint8_t*)local_name,
                                      0,
                                      NULL,
                                      0,
                                      0);
    /* Send Advertising data */
    result = aci_gap_update_adv_data(sizeof(manuf_data), (uint8_t*) manuf_data);

    BleApplicationContext.SmartPhone_Connection_Status = APP_BLE_FAST_ADV;
    if (result == BLE_STATUS_SUCCESS)
    {
    /* USER CODE BEGIN BLE_CONNECT_SUCCESS */

    /* USER CODE END BLE_CONNECT_SUCCESS */
      APP_DBG_MSG("  \r\n\r");
      APP_DBG_MSG("** START ADVERTISING **  \r\n\r");

      scannedDevicesPackage.iterator = 0;
    }
    else
    {
    /* USER CODE BEGIN BLE_CONNECT_FAILED */
      BSP_LED_On(LED_RED);
    /* USER CODE END BLE_CONNECT_FAILED */
      APP_DBG_MSG("BLE_APP_Adv_Request(), Failed \r\n\r");
    }
  }
  /* USER CODE BEGIN Connect_Request_2 */

  /* USER CODE END Connect_Request_2 */
  return;
}

/**
 * @brief  Connection Establishement on SERVER 1
 * @param  None
 * @retval None
 */
static void ConnReq1( void )
{
  tBleStatus result;
  APP_DBG_MSG("\r\n\r** CREATE CONNECTION TO END DEVICE 1 **  \r\n\r");
  if (usedDeviceInformations[0].state != APP_BLE_CONNECTED_CLIENT && usedDeviceInformations[0].isNotEmpty == true)
  {
    /* USER CODE BEGIN APP_BLE_CONNECTED_SUCCESS_END_DEVICE_1 */

    /* USER CODE END APP_BLE_CONNECTED_SUCCESS_END_DEVICE_1 */
        result = aci_gap_create_connection(
        SCAN_P,
        SCAN_L,
		RANDOM_ADDR,
		usedDeviceInformations[0].macAddress,
        PUBLIC_ADDR,
        CONN_P(50),
        CONN_P(100),
        0,
        500,
        CONN_L(10),
        CONN_L(10));

    if (result == BLE_STATUS_SUCCESS)
    {
    /* USER CODE BEGIN BLE_STATUS_END_DEVICE_1_SUCCESS */
    	int index = getCorrespondingIndex(usedDeviceInformations[0].macAddress);

    /* USER CODE END BLE_STATUS_END_DEVICE_1_SUCCESS */
    	usedDeviceInformations[0].state  = APP_BLE_LP_CONNECTING;
    	scannedDevicesPackage.scannedDevicesList[index].pairingStatus = usedDeviceInformations[0].state;
    }
    else
    {
    /* USER CODE BEGIN BLE_STATUS_END_DEVICE_1_FAILED */
      BSP_LED_On(LED_RED);
    /* USER CODE END BLE_STATUS_END_DEVICE_1_FAILED */
      usedDeviceInformations[0].state  = APP_BLE_IDLE;
    }
  }

  return;
}

#if (CFG_P2P_DEMO_MULTI != 0)
    /* USER CODE BEGIN ConnReq1_Multi */
/**
 * @brief  Connection Establishement on SERVER 2
 * @param  None
 * @retval None
 */
static void ConnReq2( void )
{
  tBleStatus result;
  APP_DBG_MSG("\r\n\r** CREATE CONNECTION TO END DEVICE 2 **  \r\n\r");

  if (usedDeviceInformations[1].state != APP_BLE_CONNECTED_CLIENT && usedDeviceInformations[1].isNotEmpty == true)
  {


    result = aci_gap_create_connection(
        SCAN_P,
        SCAN_L,
		STATIC_RANDOM_ADDR,
		usedDeviceInformations[1].macAddress,
        PUBLIC_ADDR,
        CONN_P(50),
        CONN_P(100),
        0,
        500,
        CONN_L(10),
        CONN_L(10));

    if (result == BLE_STATUS_SUCCESS)
    {
    	int index = getCorrespondingIndex(usedDeviceInformations[1].macAddress);

    	usedDeviceInformations[1].state = APP_BLE_LP_CONNECTING;
        scannedDevicesPackage.scannedDevicesList[index].pairingStatus = usedDeviceInformations[1].state ;

    }
    else
    {
      BSP_LED_On(LED_RED);
      usedDeviceInformations[1].state = APP_BLE_IDLE;

    }
  }

  return;
}

/**
 * @brief  Connection Establishement on SERVER 3
 * @param  None
 * @retval None
 */
static void ConnReq3( void )
{
  tBleStatus result;
  APP_DBG_MSG("\r\n\r** CREATE CONNECTION TO END DEVICE 3 **  \r\n\r");
  if (usedDeviceInformations[2].state != APP_BLE_CONNECTED_CLIENT && usedDeviceInformations[2].isNotEmpty == true)
  {

    result = aci_gap_create_connection(
        SCAN_P,
        SCAN_L,
		RANDOM_ADDR,
		usedDeviceInformations[2].macAddress,
        PUBLIC_ADDR,
        CONN_P(50),
        CONN_P(100),
        0,
        500,
        CONN_L(10),
        CONN_L(10));

    if (result == BLE_STATUS_SUCCESS)
    {
    	int index = getCorrespondingIndex(usedDeviceInformations[2].macAddress);

    	usedDeviceInformations[2].state = APP_BLE_LP_CONNECTING;
        scannedDevicesPackage.scannedDevicesList[index].pairingStatus = usedDeviceInformations[2].state;

    }
    else
    {
      BSP_LED_On(LED_RED);
      usedDeviceInformations[2].state = APP_BLE_IDLE;

    }
  }

  return;
}

/**
 * @brief  Connection Establishement on SERVER 4
 * @param  None
 * @retval None
 */
static void ConnReq4( void )
{
  tBleStatus result;
  APP_DBG_MSG("\r\n\r** CREATE CONNECTION TO END DEVICE 4 **  \r\n\r");
  if (usedDeviceInformations[3].state != APP_BLE_CONNECTED_CLIENT && usedDeviceInformations[3].isNotEmpty == true)
  {

    result = aci_gap_create_connection(
        SCAN_P,
        SCAN_L,
		RANDOM_ADDR,
		usedDeviceInformations[3].macAddress,
        PUBLIC_ADDR,
        CONN_P(50),
        CONN_P(100),
        0,
        500,
        CONN_L(10),
        CONN_L(10));

    if (result == BLE_STATUS_SUCCESS)
    {
    	int index = getCorrespondingIndex(usedDeviceInformations[3].macAddress);

    	usedDeviceInformations[3].state = APP_BLE_LP_CONNECTING;
        scannedDevicesPackage.scannedDevicesList[index].pairingStatus = usedDeviceInformations[3].state;

    }
    else
    {
      BSP_LED_On(LED_RED);
      usedDeviceInformations[3].state = APP_BLE_IDLE;

    }
  }

  return;
}

    /* USER CODE END ConnReq1_Multi */
#endif

/**
 * @brief  P2P GAP Notification
 * @param  GAP Notification (Opcode & Data)
 * @retval None
 */
void Evt_Notification( P2P_ConnHandle_Not_evt_t *pNotification )
{
/* USER CODE BEGIN Evt_Notification_1 */

/* USER CODE END Evt_Notification_1 */
  P2PR_APP_Device_Status_t device_status = { 0 };

  switch (pNotification->P2P_Evt_Opcode)
  {
    /* USER CODE BEGIN P2P_Evt_Opcode */

    /* USER CODE END P2P_Evt_Opcode */
    case SMART_PHONE1_CONN_HANDLE_EVT:
    	BSP_LED_On(LED_BLUE);
        /*EDS_STM_Update_Char(0x0001,
                (uint8_t *)&scannedDevicesPackage.numberOfScannedDevices);*/
      break;

    case P2P_SERVER1_CONN_HANDLE_EVT:
      device_status.Device1_Status = 0x81; /* Connected */
      //UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_1_ID, CFG_SCH_PRIO_0);
      break;

    case P2P_SERVER1_DISCON_HANDLE_EVT:
      device_status.Device1_Status = 0x80; /* Not connected */
      /* restart Create Connection */
      UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_1_ID, CFG_SCH_PRIO_0);
      break;

    case SMART_PHONE1_DISCON_HANDLE_EVT:
      BleApplicationContext.SmartPhone_Connection_Status = APP_BLE_IDLE;
      BSP_LED_Off(LED_BLUE);
      UTIL_SEQ_SetTask(1 << CFG_TASK_START_ADV_ID, CFG_SCH_PRIO_0);
      break;

#if (CFG_P2P_DEMO_MULTI != 0)
    /* USER CODE BEGIN P2P_SERVER_CONN_HANDLE_EVT_Multi_Notification */
    case P2P_SERVER2_CONN_HANDLE_EVT:
      device_status.Device2_Status = 0x81; /* Connected */
      UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_2_ID, CFG_SCH_PRIO_0);
      break;

    case P2P_SERVER3_CONN_HANDLE_EVT:
      device_status.Device3_Status = 0x81; /* connected */
      UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_3_ID, CFG_SCH_PRIO_0);
      break;

    case P2P_SERVER4_CONN_HANDLE_EVT:
      device_status.Device4_Status = 0x81; /* Connected */
      UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_4_ID, CFG_SCH_PRIO_0);
      break;

    /* USER CODE END P2P_SERVER_CONN_HANDLE_EVT_Multi_Notification */
    /* USER CODE BEGIN P2P_SERVER_DISCON_HANDLE_EVT_Multi_Notification */
    case P2P_SERVER2_DISCON_HANDLE_EVT:
      UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_2_ID, CFG_SCH_PRIO_0);
      break;

    case P2P_SERVER3_DISCON_HANDLE_EVT:
      device_status.Device3_Status = 0x80; /* Not connected */
      /* Restart Create Connection */
      UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_3_ID, CFG_SCH_PRIO_0);
      break;

    case P2P_SERVER4_DISCON_HANDLE_EVT:
      device_status.Device4_Status = 0x80; /* Not connected */
      UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_4_ID, CFG_SCH_PRIO_0);
      break;

    /* USER CODE END P2P_SERVER_DISCON_HANDLE_EVT_Multi_Notification */
#endif

    default:
    /* USER CODE BEGIN P2P_Evt_Opcode_Default */

    /* USER CODE END P2P_Evt_Opcode_Default */
    break;
  }
/* USER CODE BEGIN Evt_Notification_2 */

/* USER CODE END Evt_Notification_2 */
  return;
}

const uint8_t* BleGetBdAddress( void )
{
  uint8_t *otp_addr;
  const uint8_t *bd_addr;
  uint32_t udn;
  uint32_t company_id;
  uint32_t device_id;

  udn = LL_FLASH_GetUDN();

  if(udn != 0xFFFFFFFF)
  {
    company_id = LL_FLASH_GetSTCompanyID();
    device_id = LL_FLASH_GetDeviceID();

    bd_addr_udn[0] = (uint8_t)(udn & 0x000000FF);
    bd_addr_udn[1] = (uint8_t)( (udn & 0x0000FF00) >> 8 );
    bd_addr_udn[2] = (uint8_t)( (udn & 0x00FF0000) >> 16 );
    bd_addr_udn[3] = (uint8_t)device_id;
    bd_addr_udn[4] = (uint8_t)(company_id & 0x000000FF);;
    bd_addr_udn[5] = (uint8_t)( (company_id & 0x0000FF00) >> 8 );

    bd_addr = (const uint8_t *)bd_addr_udn;
  }
  else
  {
    otp_addr = OTP_Read(0);
    if(otp_addr)
    {
      bd_addr = ((OTP_ID0_t*)otp_addr)->bd_address;
    }
    else
    {
      bd_addr = M_bd_addr;
    }

  }

  return bd_addr;
}
/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */

/* USER CODE END FD_LOCAL_FUNCTIONS */
/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/
void hci_notify_asynch_evt(void* pdata)
{
  UTIL_SEQ_SetTask(1 << CFG_TASK_HCI_ASYNCH_EVT_ID, CFG_SCH_PRIO_0);
  return;
}

void hci_cmd_resp_release(uint32_t flag)
{
  UTIL_SEQ_SetEvt(1 << CFG_IDLEEVT_HCI_CMD_EVT_RSP_ID);
  return;
}

void hci_cmd_resp_wait(uint32_t timeout)
{
  UTIL_SEQ_WaitEvt(1 << CFG_IDLEEVT_HCI_CMD_EVT_RSP_ID);
  return;
}

static void BLE_UserEvtRx( void * pPayload )
{
  SVCCTL_UserEvtFlowStatus_t svctl_return_status;
  tHCI_UserEvtRxParam *pParam;

  pParam = (tHCI_UserEvtRxParam *)pPayload;

  svctl_return_status = SVCCTL_UserEvtRx((void *)&(pParam->pckt->evtserial));
  if (svctl_return_status != SVCCTL_UserEvtFlowDisable)
  {
    pParam->status = HCI_TL_UserEventFlow_Enable;
  }
  else
  {
    pParam->status = HCI_TL_UserEventFlow_Disable;
  }
}

static void BLE_StatusNot( HCI_TL_CmdStatus_t status )
{
  uint32_t task_id_list;
  switch (status)
  {
    case HCI_TL_CmdBusy:
      /**
       * All tasks that may send an aci/hci commands shall be listed here
       * This is to prevent a new command is sent while one is already pending
       */
      task_id_list = (1 << CFG_LAST_TASK_ID_WITH_HCICMD) - 1;
      UTIL_SEQ_PauseTask(task_id_list);

      break;

    case HCI_TL_CmdAvailable:
      /**
       * All tasks that may send an aci/hci commands shall be listed here
       * This is to prevent a new command is sent while one is already pending
       */
      task_id_list = (1 << CFG_LAST_TASK_ID_WITH_HCICMD) - 1;
      UTIL_SEQ_ResumeTask(task_id_list);

      break;

    default:
      break;
  }
  return;
}

void SVCCTL_ResumeUserEventFlow( void )
{
  hci_resume_flow();
  return;
}

/* USER CODE BEGIN FD_WRAP_FUNCTIONS */

/* USER CODE END FD_WRAP_FUNCTIONS */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
