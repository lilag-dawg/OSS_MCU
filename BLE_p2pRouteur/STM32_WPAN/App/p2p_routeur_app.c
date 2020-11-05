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

}P2P_ClientContext_t;

/* USER CODE BEGIN PTD */

typedef struct{
	uint8_t Value;
	uint8_t CurrentPosition;
}gestion_conn_NbrSensor_t;


typedef struct{
	uint8_t Pairing;
	char SensorName[19];
}gestion_conn_PairingRequest_t;

typedef struct
{

    gestion_conn_NbrSensor_t 		NumberOfSensorNearbyStruct;
    gestion_conn_PairingRequest_t	PairingRequestStruct;

    uint8_t Update_timer_Id;
   

} P2P_Router_App_Context_t;

/* Private defines ------------------------------------------------------------*/
#define UNPACK_2_BYTE_PARAMETER(ptr)  \
        (uint16_t)((uint16_t)(*((uint8_t *)ptr))) |   \
        (uint16_t)((((uint16_t)(*((uint8_t *)ptr + 1))) << 8))

#define MAX_NAMES							8
#define MAX_CARAC							19


#define NAME_CHANGES_PERIODE			(0.1*1000*1000/CFG_TS_TICK_VAL) //100ms//
/* USER CODE BEGIN PD */

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
static void Client_Update_Service( void );
void P2P_Router_APP_Init(void);
void P2P_Client_App_Notification(P2P_Client_App_Notification_evt_t *pNotification);
void P2P_Client_Init(void);
/* USER CODE BEGIN PFP */


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
        case EDS_STM_NOTIFY_ENABLED_EVT:
#if(CFG_DEBUG_APP_TRACE != 0)
            APP_DBG_MSG("-- APPLICATION ROUTER : ENDDEVICEMGT NOTIFICATION ENABLED\r\n");
#endif
            /* USER CODE BEGIN EDS_STM_NOTIFY_ENABLED_EVT */
            //HW_TS_Start(P2P_Router_App_Context.Update_timer_Id, NAME_CHANGES_PERIODE);
            /* USER CODE END EDS_STM_NOTIFY_ENABLED_EVT */
            break;

        case EDS_STM_NOTIFY_DISABLED_EVT:
#if(CFG_DEBUG_APP_TRACE != 0)
            APP_DBG_MSG("-- APPLICATION ROUTER : ENDDEVICEMGT NOTIFICATION DISABLED\r\n");
#endif
            /* USER CODE BEGIN EDS_STM_NOTIFY_DISABLED_EVT */
            //HW_TS_Stop(P2P_Router_App_Context.Update_timer_Id);
            /* USER CODE END EDS_STM_NOTIFY_DISABLED_EVT */
            break;
        case EDS_STM_WRITE_EVT:
#if(CFG_DEBUG_APP_TRACE != 0)
        	APP_DBG_MSG("-- GATT : WRITE CHAR INFO RECEIVED\n");
#endif
            /* USER CODE BEGIN EDS_STM_WRITE_EVT */

        	P2P_Router_App_Context.PairingRequestStruct.Pairing = pNotification->DataTransfered.pPayload[0];

            for(int i=0; i<(sizeof(P2P_Router_App_Context.PairingRequestStruct.SensorName));i++){
            	P2P_Router_App_Context.PairingRequestStruct.SensorName[i] = 0;
            }// reset à 0 avant chaque nouvelle lecture

            for(int i=1; i<pNotification->DataTransfered.Length;i++){
            	P2P_Router_App_Context.PairingRequestStruct.SensorName[i-1] = pNotification->DataTransfered.pPayload[i];
            	printf("%c", P2P_Router_App_Context.PairingRequestStruct.SensorName[i]);
            }

            /* USER CODE END EDS_STM_WRITE_EVT */
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

    //HW_TS_Create(CFG_TIM_PROC_ID_ISR, &(P2P_Router_App_Context.Update_timer_Id), hw_ts_Repeated, P2P_SensorName_Timer_Callback);

    /* USER CODE BEGIN P2P_Router_APP_Init_2 */
    /**
     * Initialize LedButton Service
     */

    //tempo
    P2P_Router_App_Context.NumberOfSensorNearbyStruct.CurrentPosition = 0;

    P2P_Router_App_Context.PairingRequestStruct.Pairing = 0;
    for(int i=0; i<(sizeof(P2P_Router_App_Context.PairingRequestStruct.SensorName));i++){
    	P2P_Router_App_Context.PairingRequestStruct.SensorName[i] = 0;
    }


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

/* USER CODE END P2P_Client_App_Notification_1 */
    switch(pNotification->P2P_Client_Evt_Opcode)
    {
    /* USER CODE BEGIN P2P_Client_Evt_Opcode */

    /* USER CODE END P2P_Client_Evt_Opcode */
        default:
    /* USER CODE BEGIN P2P_Client_Evt_Opcode_default */

    /* USER CODE END P2P_Client_Evt_Opcode_default */
            break;

    }
/* USER CODE BEGIN P2P_Client_App_Notification_2 */

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
static void Client_Update_Service( void )
{
	  uint16_t enable = 0x0001;
	  uint16_t disable = 0x0000;

	  uint8_t index;

	  index = 0;
	  while((index < BLE_CFG_CLT_MAX_NBR_CB) &&
	          (aP2PClientContext[index].state != APP_BLE_IDLE))
	  {


	    switch(aP2PClientContext[index].state)
	    {

	      case APP_BLE_DISCOVER_SERVICES:
	        APP_DBG_MSG("P2P_DISCOVER_SERVICES\n");
	        break;
	      case APP_BLE_DISCOVER_CHARACS:
	        APP_DBG_MSG("* GATT : Discover P2P Characteristics\n");
	        aci_gatt_disc_all_char_of_service(aP2PClientContext[index].connHandle,
	                                          aP2PClientContext[index].P2PServiceHandle,
	                                          aP2PClientContext[index].P2PServiceEndHandle);

	        break;
	      case APP_BLE_DISCOVER_WRITE_DESC: /* Not Used - No decriptor */
	        APP_DBG_MSG("* GATT : Discover Descriptor of TX - Write Characteritic\n");
	        aci_gatt_disc_all_char_desc(aP2PClientContext[index].connHandle,
	                                    aP2PClientContext[index].P2PWriteToServerCharHdle,
	                                    aP2PClientContext[index].P2PWriteToServerCharHdle+2);

	        break;
	      case APP_BLE_DISCOVER_NOTIFICATION_CHAR_DESC:
	        APP_DBG_MSG("* GATT : Discover Descriptor of Rx - Notification Characteritic\n");
	        aci_gatt_disc_all_char_desc(aP2PClientContext[index].connHandle,
	                                    aP2PClientContext[index].P2PNotificationCharHdle,
	                                    aP2PClientContext[index].P2PNotificationCharHdle+2);

	        break;
	      case APP_BLE_ENABLE_NOTIFICATION_DESC:
	        APP_DBG_MSG("* GATT : Enable Server Notification\n");
	        aci_gatt_write_char_desc(aP2PClientContext[index].connHandle,
	                                 aP2PClientContext[index].P2PNotificationDescHandle,
	                                 2,
	                                 (uint8_t *)&enable);

	        aP2PClientContext[index].state = APP_BLE_CONNECTED_CLIENT;
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


/**
 * @brief  Event handler
 * @param  Event: Address of the buffer holding the Event
 * @retval Ack: Return whether the Event has been managed or not
 */
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

                    /* USER CODE END EVT_BLUE_ATT_READ_BY_GROUP_TYPE_RESP */
                    aci_att_read_by_group_type_resp_event_rp0 *pr = (void*)blue_evt->data;
                    uint8_t numServ, i, idx;
                    uint16_t uuid, handle;

                    uint8_t index;
                    handle = pr->Connection_Handle;
                    index = 0;
                    while((index < BLE_CFG_CLT_MAX_NBR_CB) &&
                            (aP2PClientContext[index].state != APP_BLE_IDLE)/* &&
                        (aP2PClientContext[index].connHandle != handle)*/)
                    {
                        APP_BLE_ConnStatus_t status;

                        status = APP_BLE_Get_Client_Connection_Status(aP2PClientContext[index].connHandle);
                        if((aP2PClientContext[index].state == APP_BLE_CONNECTED_CLIENT)&&
                                (status == APP_BLE_IDLE))
                        {
                            /* Handle deconnected */

                            aP2PClientContext[index].state = APP_BLE_IDLE;
                            aP2PClientContext[index].connHandle = 0xFFFF;
                            break;
                        }
                        index++;
                    }

                    if(index < BLE_CFG_CLT_MAX_NBR_CB)
                    {
                        aP2PClientContext[index].connHandle= handle;
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
#if (UUID_128BIT_FORMAT==1)
                        if (pr->Attribute_Data_Length == 20)
                        {
                            idx = 16;
#else
                            if (pr->Attribute_Data_Length == 6)
                            {
                                idx = 4;
#endif
                                for (i=0; i<numServ; i++)
                                {
                                    uuid = UNPACK_2_BYTE_PARAMETER(&pr->Attribute_Data_List[idx]);
                                    if(uuid == SENSOR_SERVICE_UUID)
                                    {
#if(CFG_DEBUG_APP_TRACE != 0)
                                        APP_DBG_MSG("-- GATT : SENSOR_SERVICE_UUID FOUND - connection handle 0x%x \n", aP2PClientContext[index].connHandle);
#endif
                                        #if (UUID_128BIT_FORMAT==1)
                                        aP2PClientContext[index].P2PServiceHandle = UNPACK_2_BYTE_PARAMETER(&pr->Attribute_Data_List[idx-16]);
                                        aP2PClientContext[index].P2PServiceEndHandle = UNPACK_2_BYTE_PARAMETER (&pr->Attribute_Data_List[idx-14]);
#else
                                        aP2PClientContext[index].P2PServiceHandle = UNPACK_2_BYTE_PARAMETER(&pr->Attribute_Data_List[idx-4]);
                                        aP2PClientContext[index].P2PServiceEndHandle = UNPACK_2_BYTE_PARAMETER (&pr->Attribute_Data_List[idx-2]);
#endif
                                        aP2PClientContext[index].state = APP_BLE_DISCOVER_CHARACS ;
                                    }
                                    idx += 6;
                                }
                            }
                        }
                    }
                    break;

                case EVT_BLUE_ATT_READ_BY_TYPE_RESP:
                {
                    /* USER CODE BEGIN EVT_BLUE_ATT_READ_BY_TYPE_RESP */

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
                            (aP2PClientContext[index].connHandle != pr->Connection_Handle))
                        index++;

                    if(index < BLE_CFG_CLT_MAX_NBR_CB)
                    {

                        /* we are interested in only 16 bit UUIDs */
#if (UUID_128BIT_FORMAT==1)
                        idx = 17;
                        if (pr->Handle_Value_Pair_Length == 21)
#else
                            idx = 5;
                        if (pr->Handle_Value_Pair_Length == 7)
#endif
                        {
                            pr->Data_Length -= 1;
                            while(pr->Data_Length > 0)
                            {
                                uuid = UNPACK_2_BYTE_PARAMETER(&pr->Handle_Value_Pair_Data[idx]);
                                /* store the characteristic handle not the attribute handle */
#if (UUID_128BIT_FORMAT==1)
                                handle = UNPACK_2_BYTE_PARAMETER(&pr->Handle_Value_Pair_Data[idx-14]);
#else
                                handle = UNPACK_2_BYTE_PARAMETER(&pr->Handle_Value_Pair_Data[idx-2]);
#endif
                                if(uuid == SENSOR_NOTIFY_CHAR_UUID)
                                {
#if(CFG_DEBUG_APP_TRACE != 0)
                                	  APP_DBG_MSG("-- GATT : SENSOR_NOTIFY_CHAR_UUID FOUND  - connection handle 0x%x\n", aP2PClientContext[index].connHandle);
#endif
                                    aP2PClientContext[index].state = APP_BLE_DISCOVER_NOTIFICATION_CHAR_DESC;
                                    aP2PClientContext[index].P2PNotificationCharHdle = handle;
                                }
#if (UUID_128BIT_FORMAT==1)
                                pr->Data_Length -= 21;
                                idx += 21;
#else
                                pr->Data_Length -= 7;;
                                idx += 7;;
#endif
                            }
                        }
                    }
                }
                break;

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
                            (aP2PClientContext[index].connHandle != pr->Connection_Handle))

                        index++;

                    if(index < BLE_CFG_CLT_MAX_NBR_CB)
                    {

                        numDesc = (pr->Event_Data_Length) / 4;
                        /* we are interested only in 16 bit UUIDs */
                        idx = 0;
                        if (pr->Format == UUID_TYPE_16)
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
                                    if( aP2PClientContext[index].state == APP_BLE_DISCOVER_NOTIFICATION_CHAR_DESC)
                                    {

                                      aP2PClientContext[index].P2PNotificationDescHandle = handle;
                                      aP2PClientContext[index].state = APP_BLE_ENABLE_NOTIFICATION_DESC;

                                    }
                                }
                                idx += 4;
                            }
                        }
                    }
                }
                break; /*EVT_BLUE_ATT_FIND_INFORMATION_RESP*/

                case EVT_BLUE_GATT_NOTIFICATION:
                {
                    /* USER CODE BEGIN EVT_BLUE_GATT_NOTIFICATION */

                    /* USER CODE END EVT_BLUE_GATT_NOTIFICATION */
                    aci_gatt_notification_event_rp0 *pr = (void*)blue_evt->data;
                    uint8_t index;

                    index = 0;
                    while((index < BLE_CFG_CLT_MAX_NBR_CB) &&
                            (aP2PClientContext[index].connHandle != pr->Connection_Handle))
                        index++;

                    if(index < BLE_CFG_CLT_MAX_NBR_CB)
                    {

                        if ( (pr->Attribute_Handle == aP2PClientContext[index].P2PNotificationCharHdle))
                        {
#if(CFG_DEBUG_APP_TRACE != 0)
                            APP_DBG_MSG("-- GATT : BUTTON CHARACTERISTICS RECEIVED_EVT - connection handle 0x%x\n", aP2PClientContext[index].connHandle);
#endif
                            Notification.P2P_Client_Evt_Opcode = P2P_NOTIFICATION_INFO_RECEIVED_EVT;
                            Notification.DataTransfered.Length = pr->Attribute_Value_Length;
                            Notification.DataTransfered.pPayload = pr->Attribute_Value;

                            P2P_Client_App_Notification(&Notification);

                            /* INFORM APPLICATION BUTTON IS PUSHED BY END DEVICE */

                        }
                    }
                }
                break;/* end EVT_BLUE_GATT_NOTIFICATION */

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
                            (aP2PClientContext[index].connHandle != pr->Connection_Handle))
                        index++;

                    if(index < BLE_CFG_CLT_MAX_NBR_CB)
                    {
                        UTIL_SEQ_SetTask(  1<<CFG_TASK_SEARCH_SERVICE_ID, CFG_SCH_PRIO_0 );
                    }
                }
                break; /*EVT_BLUE_GATT_PROCEDURE_COMPLETE*/
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
