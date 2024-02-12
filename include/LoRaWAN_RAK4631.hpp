#ifndef LoRaWAN_RAK4631_hpp
#define LoRaWAN_RAK4631_hpp

#include <LoRaWan-Arduino.h>
#include "SCHC_Stack_L2.hpp"
#include "SCHC_Macros.hpp"

#define LORAWAN_APP_DATA_BUFF_SIZE 64                     /**< buffer size of the data to be transmitted. */

class LoRaWAN_RAK4631: public SCHC_Stack_L2
{
    public:
        LoRaWAN_RAK4631();
        virtual uint8_t initialize_stack(void);
        virtual uint8_t send_frame(uint8_t ruleID, char* msg, int len);
        static void lorawan_has_joined_handler(void);
        static void lorawan_join_failed_handler(void);
        static void lorawan_rx_handler(lmh_app_data_t *app_data);
        static void lorawan_confirm_class_handler(DeviceClass_t Class);
        static void lorawan_unconf_finished(void);
    private:
        bool _doOTAA;
        lmh_callback_t g_lora_callbacks;
        lmh_param_t g_lora_param_init;
        uint8_t* m_lora_app_data_buffer;
        lmh_app_data_t m_lora_app_data;
};


#endif