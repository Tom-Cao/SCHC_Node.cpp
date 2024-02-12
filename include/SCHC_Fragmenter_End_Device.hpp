#ifndef SCHC_Fragmenter_End_Device_hpp
#define SCHC_Fragmenter_End_Device_hpp

#include "SCHC_Macros.hpp"
#include "SCHC_Session_End_Device.hpp"
#include "LoRaWAN_RAK4631.hpp"
#include <Arduino.h>

class SCHC_Fragmenter_End_Device
{
    public:
        SCHC_Fragmenter_End_Device();
        uint8_t initialize(uint8_t protocol);
        uint8_t send(char* buffer, int len);
    private:
        int getSessionId(uint8_t direction);
        uint8_t _protocol;
        SCHC_Session_End_Device _uplinkSessionPool[_SESSION_POOL_SIZE];
        SCHC_Session_End_Device _downlinkSessionPool[_SESSION_POOL_SIZE];
        SCHC_Stack_L2* _stack;
};

#endif  // SCHC_Fragmenter_End_Device_hpp