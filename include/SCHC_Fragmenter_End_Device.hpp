#ifndef SCHC_Fragmenter_End_Device_hpp
#define SCHC_Fragmenter_End_Device_hpp

#include "SCHC_Macros.hpp"
#include "SCHC_Session_End_Device.hpp"
#include "LoRaWAN_RAK4631.hpp"
#include <Arduino.h>
#include <unordered_map>

class SCHC_Fragmenter_End_Device
{
    public:
        SCHC_Fragmenter_End_Device();
        uint8_t initialize(uint8_t protocol);
        uint8_t send(char* buffer, int len);
        uint8_t process_received_message(char* buffer, int len, int fport);
    private:
        int         get_free_session_id(uint8_t direction);
        uint8_t     associate_session_id(int rule_id, int sessionId);
        uint8_t     disassociate_session_id(int rule_id);
        int         get_session_id(int rule_id);
        uint8_t _protocol;
        SCHC_Session_End_Device*                _uplinkSessionPool[_SESSION_POOL_SIZE];
        SCHC_Session_End_Device*                _downlinkSessionPool[_SESSION_POOL_SIZE];
        SCHC_Stack_L2*                          _stack;
        std::unordered_map<int, int>            _associationMap;
};

#endif  // SCHC_Fragmenter_End_Device_hpp