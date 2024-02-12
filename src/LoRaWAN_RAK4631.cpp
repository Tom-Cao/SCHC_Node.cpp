#include "LoRaWAN_RAK4631.hpp"

LoRaWAN_RAK4631::LoRaWAN_RAK4631()
{
    m_lora_app_data_buffer[LORAWAN_APP_DATA_BUFF_SIZE];            //< Lora user application data buffer.
    m_lora_app_data = {m_lora_app_data_buffer, 0, 0, 0, 0}; //< Lora user application data structure.
}

uint8_t LoRaWAN_RAK4631::initialize_stack(void)
{
    /***** Lora and LoRaWAN Initialization *****
     * 1.- Define callbacks and initialize lmh_callback_t object
     * 2.- Define LoRa and LoRaWAN parameters and initialize lmh_param_t object
     * 3.- Setup the EUIs and Keys
     * 4.- Initialize LoRa chip
     * 5.- Print LoRaWAN parameters
     * 6.- Initialize LoRaWAN
     * 7.- Start Join procedure
     * 8.- Private definitions
    */

    // ************** Callbacks ******************
    //void lorawan_has_joined_handler(void);
    //void lorawan_join_failed_handler(void);
    //void lorawan_rx_handler(lmh_app_data_t *app_data);
    //void lorawan_confirm_class_handler(DeviceClass_t Class);
    //void send_lora_frame(void);

    g_lora_callbacks  = {BoardGetBatteryLevel, 
                                            BoardGetUniqueId, 
                                            BoardGetRandomSeed,
                                            this->lorawan_rx_handler, 
                                            this->lorawan_has_joined_handler, 
                                            this->lorawan_confirm_class_handler, 
                                            this->lorawan_join_failed_handler,
                                            this->lorawan_unconf_finished
                                            };

    // ************** LoRa and LoRaWAN Params ******************
    #define LORAWAN_DATERATE            DR_1					/* LoRaWAN datarates definition, from DR_0 to DR_5*/
    #define LORAWAN_TX_POWER            TX_POWER_5				/* LoRaWAN tx power definition, from TX_POWER_0 to TX_POWER_15*/
    #define JOINREQ_NBTRIALS            3						/* *< Number of trials for the join request. */
    _doOTAA                             = true;                 /* OTAA is used by default. */
    DeviceClass_t g_CurrentClass        = CLASS_A;				/* class definition*/
    LoRaMacRegion_t g_CurrentRegion     = LORAMAC_REGION_AU915; /* Region:EU868*/

    g_lora_param_init    = {LORAWAN_ADR_OFF, 
                                        LORAWAN_DATERATE, 
                                        LORAWAN_PUBLIC_NETWORK, 
                                        JOINREQ_NBTRIALS, 
                                        LORAWAN_TX_POWER, 
                                        LORAWAN_DUTYCYCLE_OFF};

    // ************** Setup the EUIs and Keys *******************
    //OTAA keys !!!! KEYS ARE MSB !!!!
    uint8_t nodeDeviceEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x0C, 0xEF, 0xD0};
    uint8_t nodeAppEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    uint8_t nodeAppKey[16] = {0x28, 0xE4, 0xCB, 0x60, 0x68, 0x05, 0x45, 0x14, 0x96, 0xEC, 0x2D, 0x13, 0x91, 0x11, 0x82, 0xB8};

    // ABP keys
    uint32_t nodeDevAddr = 0x260116F8;
    uint8_t nodeNwsKey[16] = {0x7E, 0xAC, 0xE2, 0x55, 0xB8, 0xA5, 0xE2, 0x69, 0x91, 0x51, 0x96, 0x06, 0x47, 0x56, 0x9D, 0x23};
    uint8_t nodeAppsKey[16] = {0xFB, 0xAC, 0xB6, 0x47, 0xF3, 0x58, 0x45, 0xC7, 0x50, 0x7D, 0xBF, 0x16, 0x8B, 0xA8, 0xC1, 0x7C};

    if (_doOTAA)
    {
        lmh_setDevEui(nodeDeviceEUI);
        lmh_setAppEui(nodeAppEUI);
        lmh_setAppKey(nodeAppKey);
    }
    else
    {
        lmh_setNwkSKey(nodeNwsKey);
        lmh_setAppSKey(nodeAppsKey);
        lmh_setDevAddr(nodeDevAddr);
    }
  
    // ************** Initialize LoRa chip *************
    lora_rak4630_init();

    // ************** Print LoRaWAN parameters *********
    Serial.println("=====================================");
    Serial.println("Welcome to RAK4630 LoRaWan!!!");
    if (_doOTAA)
    {
        Serial.println("Type: OTAA");
    }
    else
    {
        Serial.println("Type: ABP");
    }

    switch (g_CurrentRegion)
    {
        case LORAMAC_REGION_AS923:
            Serial.println("Region: AS923");
            break;
        case LORAMAC_REGION_AU915:
            Serial.println("Region: AU915");
            break;
        case LORAMAC_REGION_CN470:
            Serial.println("Region: CN470");
            break;
        case LORAMAC_REGION_CN779:
            Serial.println("Region: CN779");
            break;
        case LORAMAC_REGION_EU433:
            Serial.println("Region: EU433");
            break;
        case LORAMAC_REGION_IN865:
            Serial.println("Region: IN865");
            break;
        case LORAMAC_REGION_EU868:
            Serial.println("Region: EU868");
            break;
        case LORAMAC_REGION_KR920:
            Serial.println("Region: KR920");
            break;
        case LORAMAC_REGION_US915:
            Serial.println("Region: US915");
            break;
        case LORAMAC_REGION_RU864:
            Serial.println("Region: RU864");
            break;
        case LORAMAC_REGION_AS923_2:
            Serial.println("Region: AS923-2");
            break;
        case LORAMAC_REGION_AS923_3:
            Serial.println("Region: AS923-3");
            break;
        case LORAMAC_REGION_AS923_4:
            Serial.println("Region: AS923-4");
            break;
    }
    Serial.println("=====================================");


    // ************** Initialize LoRaWAN ***************
    uint32_t err_code;
    err_code = lmh_init(&g_lora_callbacks,  
                        g_lora_param_init, 
                        _doOTAA, 
                        g_CurrentClass, 
                        g_CurrentRegion);
    if (err_code != 0)
    {
        char myBuffer[200];
        sprintf(myBuffer, "lmh_init failed - %d\n", (int)err_code);
        Serial.println(myBuffer);
        return 1;
    } 

    // ************** Start Join procedure *************
    lmh_join();

    return 0;
}

uint8_t LoRaWAN_RAK4631::send_frame(uint8_t ruleID, char* msg, int len)
{

    // ************** Private definitions **************
    memset(m_lora_app_data.buffer, 0, LORAWAN_APP_DATA_BUFF_SIZE);
    lmh_confirm g_CurrentConfirm        = LMH_UNCONFIRMED_MSG;  /* confirm/unconfirm packet definition*/
    uint8_t gAppPort                    = ruleID;     /* data port*/

    if (lmh_join_status_get() != LMH_SET)
    {
        //Not joined, try again later
        Serial.println("Not joined, try again later");
        return 1;
    }

    m_lora_app_data.port = gAppPort;
    m_lora_app_data.buffsize = len;
    for(int i=0; i<len; i++)
    {
        m_lora_app_data.buffer[i] = (uint8_t)msg[i];
    }

    lmh_error_status error = lmh_send_blocking(&m_lora_app_data, g_CurrentConfirm,10000);
    if (error == LMH_SUCCESS)
    {
        char myBuffer[30];
        sprintf(myBuffer, "lmh_send ok: %d",error);
#ifndef MYINFO
        Serial.println(myBuffer);
#endif
        return 0;
    }
    else if(error == LMH_BUSY)
    {
        char myBuffer[30];
        sprintf(myBuffer, "lmh_send busy: %d",error);
#ifndef MYINFO
        Serial.println(myBuffer);
#endif
        return 1;
    }
    else if(error == LMH_ERROR)
    {
        char myBuffer[30];
        sprintf(myBuffer, "lmh_send error: %d",error);
#ifndef MYINFO
        Serial.println(myBuffer);
#endif
        return 1;
    }


     
}

void LoRaWAN_RAK4631::lorawan_has_joined_handler(void)
{
  Serial.println("Network Joined!");
}

void LoRaWAN_RAK4631::lorawan_join_failed_handler(void)
{
  Serial.println("OTAA join failed!");
  Serial.println("Check your EUI's and Keys's!");
  Serial.println("Check if a Gateway is in range!");
}

void LoRaWAN_RAK4631::lorawan_rx_handler(lmh_app_data_t *app_data)
{
    char myBuffer[200];
    sprintf(myBuffer, "LoRa Packet received on port %d, size:%d, rssi:%d, snr:%d, data:%s\n",app_data->port, app_data->buffsize, app_data->rssi, app_data->snr, app_data->buffer);
    Serial.println(myBuffer);
}

void LoRaWAN_RAK4631::lorawan_confirm_class_handler(DeviceClass_t Class)
{
    char myBuffer[200];
    sprintf(myBuffer, "switch to class %c done\n", "ABC"[Class]);
    Serial.println(myBuffer);
}

void LoRaWAN_RAK4631::lorawan_unconf_finished(void)
{
#ifndef MYINFO
    Serial.println("TX unconfirmed finished!!");
#endif
}
