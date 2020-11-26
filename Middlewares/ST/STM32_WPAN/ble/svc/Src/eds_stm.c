/**
  ******************************************************************************
  * @file    eds_stm.c
  * @author  MCD Application Team
  * @brief   End Device Service (Custom - STM)
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


/* Includes ------------------------------------------------------------------*/
#include "common_blesvc.h"


/* Private typedef -----------------------------------------------------------*/
typedef struct{
  uint16_t	EndDeviceManagementSvcHdle;				/**< Service handle */
  uint16_t	EndDeviceStatusCharHdle;	                /**< Characteristic handle */
  uint16_t	EndDeviceStatusWriteClientToServerCharHdle;	  /**< Characteristic handle */
  uint16_t	EndDeviceStatusNotifyServerToClientCharHdle1;	/**< Characteristic handle */
  uint16_t	EndDeviceStatusNotifyServerToClientCharHdle2;	/**< Characteristic handle */

}EndDeviceManagementContext_t;



/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
PLACE_IN_SECTION("BLE_DRIVER_CONTEXT") static EndDeviceManagementContext_t aEndDeviceManagementContext;
PLACE_IN_SECTION("BLE_DRIVER_CONTEXT") static EndDeviceManagementContext_t bEndDeviceManagementContext;

/* Private function prototypes -----------------------------------------------*/
static SVCCTL_EvtAckStatus_t EndDeviceManagement_Event_Handler(void *pckt);


/* Functions Definition ------------------------------------------------------*/
/* Private functions ----------------------------------------------------------*/
#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
do {\
    uuid_struct[0] = uuid_0; uuid_struct[1] = uuid_1; uuid_struct[2] = uuid_2; uuid_struct[3] = uuid_3; \
        uuid_struct[4] = uuid_4; uuid_struct[5] = uuid_5; uuid_struct[6] = uuid_6; uuid_struct[7] = uuid_7; \
            uuid_struct[8] = uuid_8; uuid_struct[9] = uuid_9; uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
                uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
}while(0)

#define COPY_UUID_16(uuid_struct, uuid_0) \
do {\
    uuid_struct = uuid_0;\
}while(0)

/* Hardware Characteristics Service */

#define COPY_EDM_CONNEX_HAND_SERVICE_UUID(uuid_struct)   COPY_UUID_16(uuid_struct,0x1802)
#define COPY_EDM_CONNEX_HAND_CARA_1_UUID(uuid_struct)	 COPY_UUID_16(uuid_struct,0x16A1)
#define COPY_EDM_CONNEX_HAND_CARA_2_UUID(uuid_struct)    COPY_UUID_16(uuid_struct,0x16A2)
#define COPY_EDM_CONNEX_HAND_CARA_3_UUID(uuid_struct)    COPY_UUID_16(uuid_struct,0x16A3)
#define COPY_EDM_CONNEX_HAND_CARA_4_UUID(uuid_struct)    COPY_UUID_16(uuid_struct,0x16A4)

#define COPY_EDM_CALIBRATION_SERVICE_UUID(uuid_struct)   COPY_UUID_16(uuid_struct,0x1803)
#define COPY_EDM_CALIBRATION_CARA_1_UUID(uuid_struct)    COPY_UUID_16(uuid_struct,0x16B1)

/**
 * @brief  Event handler
 * @param  Event: Address of the buffer holding the Event
 * @retval Ack: Return whether the Event has been managed or not
 */
static SVCCTL_EvtAckStatus_t EndDeviceManagement_Event_Handler(void *Event)
{
  SVCCTL_EvtAckStatus_t return_value;
  hci_event_pckt *event_pckt;
  evt_blue_aci *blue_evt;
  aci_gatt_attribute_modified_event_rp0    * attribute_modified;
  EDS_STM_App_Notification_evt_t Notification;

  return_value = SVCCTL_EvtNotAck;
  event_pckt = (hci_event_pckt *)(((hci_uart_pckt*)Event)->data);

  switch(event_pckt->evt)
  {
    case EVT_VENDOR:
    {
      blue_evt = (evt_blue_aci*)event_pckt->data;
      switch(blue_evt->ecode)
      {
        case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED:
       {
          attribute_modified = (aci_gatt_attribute_modified_event_rp0*)blue_evt->data;
          
            if(attribute_modified->Attr_Handle == (aEndDeviceManagementContext.EndDeviceStatusNotifyServerToClientCharHdle1 + 2))
            {
              /**
               * Descriptor handle
               */
              return_value = SVCCTL_EvtAckFlowEnable;
              /**
               * Notify to application
               */
              if(attribute_modified->Attr_Data[0] & COMSVC_Notification)
              {
               
                Notification.EDS_Evt_Opcode = EDS_CONNEX_HAND_CARA_2_NOTIFY_ENABLED_EVT;
                EDS_STM_App_Notification(&Notification);

              }
              else
              {

                Notification.EDS_Evt_Opcode = EDS_CONNEX_HAND_CARA_2_NOTIFY_DISABLED_EVT;
                EDS_STM_App_Notification(&Notification);

              }
            }
            else if(attribute_modified->Attr_Handle == (aEndDeviceManagementContext.EndDeviceStatusNotifyServerToClientCharHdle2 + 2))
            {
              /**
               * Descriptor handle
               */
              return_value = SVCCTL_EvtAckFlowEnable;
              /**
               * Notify to application
               */
              if(attribute_modified->Attr_Data[0] & COMSVC_Notification)
              {

                Notification.EDS_Evt_Opcode = EDS_CONNEX_HAND_CARA_4_NOTIFY_ENABLED_EVT;
                EDS_STM_App_Notification(&Notification);

              }
              else
              {
              
                Notification.EDS_Evt_Opcode = EDS_CONNEX_HAND_CARA_4_NOTIFY_DISABLED_EVT;
                EDS_STM_App_Notification(&Notification);

              }
            }
            else if(attribute_modified->Attr_Handle == (aEndDeviceManagementContext.EndDeviceStatusWriteClientToServerCharHdle + 1))
            {
            	Notification.EDS_Evt_Opcode = EDS_CONNEX_HAND_CARA_3_WRITE_EVT;
                Notification.DataTransfered.Length=attribute_modified->Attr_Data_Length;
                Notification.DataTransfered.pPayload=attribute_modified->Attr_Data;
            	EDS_STM_App_Notification(&Notification);
            }
            else if(attribute_modified->Attr_Handle == (bEndDeviceManagementContext.EndDeviceStatusWriteClientToServerCharHdle + 1))
            {
            	Notification.EDS_Evt_Opcode = EDS_CALIBRATION_CARA_1_WRITE_EVT;
                Notification.DataTransfered.Length=attribute_modified->Attr_Data_Length;
                Notification.DataTransfered.pPayload=attribute_modified->Attr_Data;
            	EDS_STM_App_Notification(&Notification);
            }
        }
        break;

        default:
          break;
      }
    }
    break; /* HCI_EVT_VENDOR_SPECIFIC */

    default:
      break;
  }

  return(return_value);
}/* end SVCCTL_EvtAckStatus_t */


/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Service initialization
 * @param  None
 * @retval None
 */
void EDS_STM_Init(void)
{
  Char_UUID_t  uuid16;

  /**
   *	Register the event handler to the BLE controller
   */
  SVCCTL_RegisterSvcHandler(EndDeviceManagement_Event_Handler);
  
    /**
     *  End Device Mgt Service
     *
     * Max_Attribute_Records = 2*no_of_char + 1
     * service_max_attribute_record = 1 for End Device Mgt service +
     *                                2 for End Device Staus characteristic +
     *                                1 for client char configuration descriptor 
     *                                
     */
  //for service connexion handling
  	COPY_EDM_CONNEX_HAND_SERVICE_UUID(uuid16.Char_UUID_16);
    aci_gatt_add_service(UUID_TYPE_16,
                      (Service_UUID_t *) &uuid16,
                      PRIMARY_SERVICE,
                      12,
                      &(aEndDeviceManagementContext.EndDeviceManagementSvcHdle));

    /**
     *  Add Read Characteristic
     */
    COPY_EDM_CONNEX_HAND_CARA_1_UUID(uuid16.Char_UUID_16);
    aci_gatt_add_char(aEndDeviceManagementContext.EndDeviceManagementSvcHdle,
                      UUID_TYPE_16,
                      &uuid16,
                      1,
					  CHAR_PROP_READ,
                      ATTR_PERMISSION_NONE,
                      GATT_DONT_NOTIFY_EVENTS, /* gattEvtMask */
                      10, /* encryKeySize */
                      0, /* isVariable */
                      &(aEndDeviceManagementContext.EndDeviceStatusCharHdle));

    /**
     *  Add Notify Characteristic 1
     */
    COPY_EDM_CONNEX_HAND_CARA_2_UUID(uuid16.Char_UUID_16);
    aci_gatt_add_char(aEndDeviceManagementContext.EndDeviceManagementSvcHdle,
                      UUID_TYPE_16,
                      &uuid16,
                      20,
                      CHAR_PROP_NOTIFY,
                      ATTR_PERMISSION_NONE,
					  GATT_NOTIFY_ATTRIBUTE_WRITE, /* gattEvtMask */
                      10, /* encryKeySize */
                      1, /* isVariable */
                      &(aEndDeviceManagementContext.EndDeviceStatusNotifyServerToClientCharHdle1));
    /**
     *  Add write Characteristic select
     */
    COPY_EDM_CONNEX_HAND_CARA_3_UUID(uuid16.Char_UUID_16);
    aci_gatt_add_char(aEndDeviceManagementContext.EndDeviceManagementSvcHdle,
                      UUID_TYPE_16,
                      &uuid16,
                      20,
					  CHAR_PROP_WRITE,
                      ATTR_PERMISSION_NONE,
					  GATT_NOTIFY_ATTRIBUTE_WRITE, /* gattEvtMask */
                      10, /* encryKeySize */
                      1, /* isVariable */
                      &(aEndDeviceManagementContext.EndDeviceStatusWriteClientToServerCharHdle));
    
    /**
     *  Add Notify Characteristic 2
     */
    COPY_EDM_CONNEX_HAND_CARA_4_UUID(uuid16.Char_UUID_16);
    aci_gatt_add_char(aEndDeviceManagementContext.EndDeviceManagementSvcHdle,
                      UUID_TYPE_16,
                      &uuid16,
                      20,
                      CHAR_PROP_NOTIFY,
                      ATTR_PERMISSION_NONE,
					  GATT_NOTIFY_ATTRIBUTE_WRITE, /* gattEvtMask */
                      10, /* encryKeySize */
                      1, /* isVariable */
                      &(aEndDeviceManagementContext.EndDeviceStatusNotifyServerToClientCharHdle2));

     BLE_DBG_EDS_STM_MSG("-- End Device Managment Service (EDMS) is added Successfully %04X\n",
                 aEndDeviceManagementContext.EndDeviceManagementSvcHdle);
     
    //for service calibration
    COPY_EDM_CALIBRATION_SERVICE_UUID(uuid16.Char_UUID_16);
    aci_gatt_add_service(UUID_TYPE_16,
                      (Service_UUID_t *) &uuid16,
                      PRIMARY_SERVICE,
                      10,
                      &(bEndDeviceManagementContext.EndDeviceManagementSvcHdle));
    /**
     *  Add write Characteristic select
     */
    COPY_EDM_CALIBRATION_CARA_1_UUID(uuid16.Char_UUID_16);
    aci_gatt_add_char(bEndDeviceManagementContext.EndDeviceManagementSvcHdle,
                      UUID_TYPE_16,
                      &uuid16,
                      20,
					  CHAR_PROP_WRITE,
                      ATTR_PERMISSION_NONE,
					  GATT_NOTIFY_ATTRIBUTE_WRITE, /* gattEvtMask */
                      10, /* encryKeySize */
                      1, /* isVariable */
                      &(bEndDeviceManagementContext.EndDeviceStatusWriteClientToServerCharHdle));
    
     

    return;
}

/**
 * @brief  Characteristic update
 * @param  UUID: UUID of the characteristic
 * @param  Service_Instance: Instance of the service to which the characteristic belongs
 * 
 */
tBleStatus EDS_STM_Update_Char(uint16_t UUID, uint8_t *pPayload)
{
  tBleStatus result = BLE_STATUS_INVALID_PARAMS;
  
 
  switch(UUID)
  {
    case END_DEVICE_STATUS_CHAR_UUID:
    
      result = aci_gatt_update_char_value(aEndDeviceManagementContext.EndDeviceManagementSvcHdle,
                             aEndDeviceManagementContext.EndDeviceStatusCharHdle,
                             0, /* charValOffset */
                             6, /* charValueLen */
                             (uint8_t *)  pPayload);
     
      break;

    case 0x0000:

        result = aci_gatt_update_char_value(aEndDeviceManagementContext.EndDeviceManagementSvcHdle,
                               aEndDeviceManagementContext.EndDeviceStatusNotifyServerToClientCharHdle1,
                               0, /* charValOffset */
                               20, /* charValueLen */
                               (uint8_t *)  pPayload);
       break;
    case 0x0001:

        result = aci_gatt_update_char_value(aEndDeviceManagementContext.EndDeviceManagementSvcHdle,
                               aEndDeviceManagementContext.EndDeviceStatusCharHdle,
                               0, /* charValOffset */
                               1, /* charValueLen */
                               (uint8_t *)  pPayload);

       break;

    case 0x0002:

        result = aci_gatt_update_char_value(aEndDeviceManagementContext.EndDeviceManagementSvcHdle,
                               aEndDeviceManagementContext.EndDeviceStatusWriteClientToServerCharHdle,
                               0, /* charValOffset */
                               20, /* charValueLen */
                               (uint8_t *)  pPayload);

       break;

    case 0x0003:

        result = aci_gatt_update_char_value(aEndDeviceManagementContext.EndDeviceManagementSvcHdle,
                               aEndDeviceManagementContext.EndDeviceStatusNotifyServerToClientCharHdle2,
                               0, /* charValOffset */
                               20, /* charValueLen */
                               (uint8_t *)  pPayload);
       break;
    case 0x0004:

        result = aci_gatt_update_char_value(bEndDeviceManagementContext.EndDeviceManagementSvcHdle,
                               bEndDeviceManagementContext.EndDeviceStatusWriteClientToServerCharHdle,
                               0, /* charValOffset */
                               20, /* charValueLen */
                               (uint8_t *)  pPayload);

       break;
    default:
      break;
  }

  return result;
}/* end BLE_SVC_LedButton_Update_Char() */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
