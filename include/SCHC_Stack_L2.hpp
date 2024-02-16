#ifndef SCHC_Stack_L2_hpp
#define SCHC_Stack_L2_hpp

#include <Arduino.h>

class SCHC_Stack_L2
{
public:
    virtual uint8_t initialize_stack(void);
    virtual uint8_t send_frame(uint8_t ruleID, char* msg, int len);
    virtual int getMtu(bool consider_Fopt);
};

#endif