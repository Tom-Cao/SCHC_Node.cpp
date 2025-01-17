#ifndef SCHC_Message_hpp
#define SCHC_Message_hpp

#include "SCHC_Macros.hpp"
#include <Arduino.h>

class SCHC_Message
{
    public:
        SCHC_Message();
        int         create_regular_fragment(uint8_t ruleID, uint8_t dtag, uint8_t w, uint8_t fcn, char *payload, int payload_len, char *buffer);
        int         create_ack_request(uint8_t ruleID, uint8_t dtag, uint8_t w, char *buffer);
        int         create_sender_abort(uint8_t ruleID, uint8_t dtag, uint8_t w, char *buffer);
        int         create_all_1_fragment(uint8_t ruleID, uint8_t dtag, uint8_t w, uint32_t rcs, char *payload, int payload_len, char *buffer);
        uint8_t     get_msg_type(uint8_t protocol, int rule_id, char *msg, int len);
        uint8_t     decodeMsg(uint8_t protocol, int rule_id, char *msg, int len);
        void        print_msg(uint8_t msgType, char *msg, int len);
        void        printBin(uint8_t val);
        uint8_t     get_c();
        uint8_t     get_w();
        void        get_bitmap(char* bitmap);
    private:
        uint8_t     _msg_type;
        uint8_t     _w;
        uint8_t     _c;
        uint8_t     _fcn;
        uint8_t     _dtag;
        int         _schc_payload_len;
        char*       _schc_payload = nullptr;
        char*       _rcs = nullptr;
        char*       _bitmap = nullptr; 
};

#endif