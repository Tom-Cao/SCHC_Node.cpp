#ifndef SCHC_Message_hpp
#define SCHC_Message_hpp

#include "SCHC_Macros.hpp"
#include <Arduino.h>

class SCHC_Message
{
    public:
        SCHC_Message();
        int createRegularFragment(uint8_t ruleID, uint8_t dtag, uint8_t w, uint8_t fcn, char *payload, int payload_len, char *buffer);
        SCHC_Message* decodeMsg(uint8_t protocol, char *msg, int len);
        void printMsg(uint8_t protocol, uint8_t msgType, char *msg, int len);
        void printBin(uint8_t val);

};

#endif