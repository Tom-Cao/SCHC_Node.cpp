#ifndef SCHC_Node_LoRaWAN_RAK4631_hpp
#define SCHC_Node_LoRaWAN_RAK4631_hpp

#include <LoRaWan-RAK4630.h>
#include "SCHC_Node_Stack_L2.hpp"
#include "SCHC_Node_Macros.hpp"
#include <vector>

class SCHC_Node_Fragmenter;

class SCHC_Node_LoRaWAN_RAK4631: public SCHC_Node_Stack_L2
{
    public:
        SCHC_Node_LoRaWAN_RAK4631();
        uint8_t     initialize_stack(void);
        uint8_t     send_frame(uint8_t ruleID, char* msg, int len);
        int         getMtu(bool consider_Fopt);
        void        set_fragmenter(SCHC_Node_Fragmenter* frag);
        static void lorawan_has_joined_handler(void);
        static void lorawan_join_failed_handler(void);
        static void lorawan_rx_handler(lmh_app_data_t *app_data);
        static void lorawan_confirm_class_handler(DeviceClass_t Class);
        static void lorawan_unconf_finished(void);
        static SCHC_Node_Fragmenter* _frag;
    private:
        bool _doOTAA;
        lmh_callback_t g_lora_callbacks;
        lmh_param_t g_lora_param_init;
        uint8_t* m_lora_app_data_buffer;
        lmh_app_data_t m_lora_app_data;

        std::vector<int>    _not_send_list = {2, 3};
        int                 _not_send_countr = 1;
};


#endif