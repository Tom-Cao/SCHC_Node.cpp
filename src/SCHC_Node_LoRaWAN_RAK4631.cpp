#include "SCHC_Node_LoRaWAN_RAK4631.hpp"
#include "SCHC_Node_Fragmenter.hpp"

SCHC_Node_Fragmenter* SCHC_Node_LoRaWAN_RAK4631::_frag = nullptr;

SCHC_Node_LoRaWAN_RAK4631::SCHC_Node_LoRaWAN_RAK4631()
{

}

uint8_t SCHC_Node_LoRaWAN_RAK4631::initialize_stack(void)
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
    #define LORAWAN_DATERATE            DR_5					/* LoRaWAN datarates definition, from DR_0 to DR_5*/
    #define LORAWAN_TX_POWER            TX_POWER_5				/* LoRaWAN tx power definition, from TX_POWER_0 to TX_POWER_15*/
    #define JOINREQ_NBTRIALS            3						/* *< Number of trials for the join request. */
    _doOTAA                             = true;                 /* OTAA is used by default. */
    DeviceClass_t g_CurrentClass        = CLASS_A;				/* class definition*/
    LoRaMacRegion_t g_CurrentRegion     = LORAMAC_REGION_US915; /* Region:EU868*/

    g_lora_param_init    = {LORAWAN_ADR_OFF, 
                                        LORAWAN_DATERATE, 
                                        LORAWAN_PUBLIC_NETWORK, 
                                        JOINREQ_NBTRIALS, 
                                        LORAWAN_TX_POWER, 
                                        LORAWAN_DUTYCYCLE_OFF};

    // ************** Setup the EUIs and Keys *******************
    //OTAA keys !!!! KEYS ARE MSB !!!!
    uint8_t nodeDeviceEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x0C, 0xEF, 0xD0};    // AC1F09FFFE0CEFD0
    uint8_t nodeAppEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};   // 0000000000000001
    uint8_t nodeAppKey[16] = {0xB1, 0xB0, 0x97, 0x92, 0x50, 0x17, 0xFB, 0xA7, 0x4F, 0xBA, 0x53, 0x69, 0x00, 0xC7, 0x6C, 0xAC};  // B1B097925017FBA74FBA536900C76CAC


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

uint8_t SCHC_Node_LoRaWAN_RAK4631::send_frame(uint8_t ruleID, char* msg, int len)
{
#ifdef MYTRACE
    Serial.println("SCHC_Node_LoRaWAN_RAK4631::send_frame - Entering the function");
#endif

    // ************** Private definitions **************
    uint8_t m_lora_app_data_buffer[len];			  ///< Lora user application data buffer.
    lmh_app_data_t m_lora_app_data = {m_lora_app_data_buffer, 0, 0, 0, 0}; ///< Lora user application data structure.

    if (lmh_join_status_get() != LMH_SET)
    {
        //Not joined, try again later
        Serial.println("Not joined, try again later");
        return 1;
    }

    //m_lora_app_data.port = LORAWAN_APP_PORT;
    m_lora_app_data.port = ruleID;
    for(int i=0; i<len; i++)
    {
        m_lora_app_data.buffer[i] = (uint8_t)msg[i];
    }
    m_lora_app_data.buffsize = len;

    lmh_error_status error = lmh_send_blocking(&m_lora_app_data, LMH_UNCONFIRMED_MSG,5000);
    if (error == LMH_SUCCESS)
    {
        char myBuffer[30];
        sprintf(myBuffer, "lmh_send ok: %d",error);
#ifdef MYTRACE
        Serial.println(myBuffer);
#endif
        return 0;
    }
    else if(error == LMH_BUSY)
    {
        char myBuffer[30];
        sprintf(myBuffer, "lmh_send busy: %d",error);
#ifdef MYINFO
        Serial.println(myBuffer);
#endif
        return 1;
    }
    else if(error == LMH_ERROR)
    {
        char myBuffer[30];
        sprintf(myBuffer, "lmh_send error: %d",error);
#ifdef MYINFO
        Serial.println(myBuffer);
#endif
        return 1;
    }


     return 0;
}

int SCHC_Node_LoRaWAN_RAK4631::getMtu(bool consider_Fopt)
{
    int fOpt = 0;   
    if(consider_Fopt)
    {
        fOpt = 15;  // 15 bytes is the max
    }
    
    if(LORAWAN_DATERATE==DR_0 || LORAWAN_DATERATE==DR_1 || LORAWAN_DATERATE==DR_2)
    {
        return 51 - fOpt;
    }
    else if(LORAWAN_DATERATE==DR_3)
    {
        return 115 - fOpt;   
    }
    else if(LORAWAN_DATERATE==DR_4 || LORAWAN_DATERATE==DR_5)
    {
        return 222 - fOpt;   
    }

    return -1;
}

void SCHC_Node_LoRaWAN_RAK4631::set_fragmenter(SCHC_Node_Fragmenter *frag)
{
    SCHC_Node_LoRaWAN_RAK4631::_frag = frag;
}

void SCHC_Node_LoRaWAN_RAK4631::lorawan_has_joined_handler(void)
{
  Serial.println("Network Joined!");
}

void SCHC_Node_LoRaWAN_RAK4631::lorawan_join_failed_handler(void)
{
  Serial.println("OTAA join failed!");
  Serial.println("Check your EUI's and Keys's!");
  Serial.println("Check if a Gateway is in range!");
}

void SCHC_Node_LoRaWAN_RAK4631::lorawan_rx_handler(lmh_app_data_t *app_data)
{
#ifdef MYTRACE
    Serial.println("SCHC_Node_LoRaWAN_RAK4631::lorawan_rx_handler - Entering function");
#endif    
    char* buff = new char[app_data->buffsize];
    memcpy(buff, app_data->buffer, app_data->buffsize);
    _frag->process_received_message(buff, app_data->buffsize, app_data->port);
#ifdef MYTRACE
        Serial.println("SCHC_Node_LoRaWAN_RAK4631::lorawan_rx_handler - Leaving function");
#endif
}

void SCHC_Node_LoRaWAN_RAK4631::lorawan_confirm_class_handler(DeviceClass_t Class)
{
    char myBuffer[200];
    sprintf(myBuffer, "switch to class %c done\n", "ABC"[Class]);
    Serial.println(myBuffer);
}

void SCHC_Node_LoRaWAN_RAK4631::lorawan_unconf_finished(void)
{
#ifdef MYTRACE
    Serial.println("TX unconfirmed finished!!");
#endif
}
