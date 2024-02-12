#ifndef SCHC_Session_End_Device_hpp
#define SCHC_Session_End_Device_hpp

#include "SCHC_Macros.hpp"
#include "SCHC_Ack_on_error.hpp"
#include "SCHC_State_Machine.hpp"
#include "SCHC_Stack_L2.hpp"
#include <Arduino.h>


class SCHC_Session_End_Device
{
    public:
        SCHC_Session_End_Device();
        uint8_t initialize(uint8_t protocol, uint8_t direction, uint8_t dTag, SCHC_Stack_L2* stack_ptr);
        uint8_t startFragmentation(char *buffer, int len);
        bool isUsed();
        uint8_t getDTag();
    private:
        uint8_t createStateMachine();
        bool _isUsed;
        uint8_t _protocol;
        uint8_t _direction;
        uint8_t _ruleID;
        uint8_t _dTag;
        uint8_t _tileSize;              // in bytes
        uint8_t _m;                     // in bits
        uint8_t _n;                     // in bits
        uint8_t _windowSize;            // in tiles
        uint8_t _t;                     // in bits
        uint8_t _maxAckReq;
        int _retransTimer;          // in seconds
        int _inactivityTimer;       // in seconds
        uint8_t _txAttemptsCounter;
        uint8_t _rxAttemptsCounter;
        int _maxMsgSize;               // in bytes
        SCHC_State_Machine* _stateMachine;
        SCHC_Stack_L2* _stack;
};

#endif
