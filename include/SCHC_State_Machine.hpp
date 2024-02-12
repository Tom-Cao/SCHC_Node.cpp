#ifndef SCHC_State_Machine_hpp
#define SCHC_State_Machine_hpp

#include "SCHC_Stack_L2.hpp"
#include <Arduino.h>

class SCHC_State_Machine
{
    public:
        virtual uint8_t init(uint8_t ruleID, uint8_t dTag, uint8_t windowSize, uint8_t tileSize, uint8_t n, uint8_t ackMode, SCHC_Stack_L2* stack_ptr);
        virtual uint8_t execute(char *msg=NULL, int len=0);
        virtual uint8_t start(char *buffer, int len);
};

#endif