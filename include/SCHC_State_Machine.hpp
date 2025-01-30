#ifndef SCHC_State_Machine_hpp
#define SCHC_State_Machine_hpp

#include "SCHC_Stack_L2.hpp"
#include <Arduino.h>
#include <vector>

class SCHC_State_Machine
{
    public:
        virtual ~SCHC_State_Machine()=0;
        virtual uint8_t init(uint8_t ruleID, uint8_t dTag, uint8_t windowSize, uint8_t tileSize, uint8_t n, uint8_t ackMode, SCHC_Stack_L2* stack_ptr, int retTimer, uint8_t ackReqAttempts) = 0;
        virtual uint8_t start(char* msg, int len) = 0;
        //virtual uint8_t enqueue_message(char* msg, int len) = 0;
};

#endif