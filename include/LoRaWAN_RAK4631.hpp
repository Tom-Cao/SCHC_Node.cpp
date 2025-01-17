#ifndef LoRaWAN_RAK4631_hpp
#define LoRaWAN_RAK4631_hpp

#include <LoRaWan-RAK4630.h>
#include "SCHC_Stack_L2.hpp"
#include "SCHC_Macros.hpp"

class SCHC_Fragmenter_End_Device;

class LoRaWAN_RAK4631: public SCHC_Stack_L2
{
    public:
        LoRaWAN_RAK4631();
        uint8_t     initialize_stack(void);
        uint8_t     send_frame(uint8_t ruleID, char* msg, int len);
        int         getMtu(bool consider_Fopt);
        void        set_fragmenter(SCHC_Fragmenter_End_Device* frag);
        static void lorawan_has_joined_handler(void);
        static void lorawan_join_failed_handler(void);
        static void lorawan_rx_handler(lmh_app_data_t *app_data);
        static void lorawan_confirm_class_handler(DeviceClass_t Class);
        static void lorawan_unconf_finished(void);
        static SCHC_Fragmenter_End_Device* _frag;
    private:
        bool _doOTAA;
        lmh_callback_t g_lora_callbacks;
        lmh_param_t g_lora_param_init;
        uint8_t* m_lora_app_data_buffer;
        lmh_app_data_t m_lora_app_data;
};


#endif