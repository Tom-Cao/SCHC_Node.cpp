#include "SCHC_Session_End_Device.hpp"

SCHC_Session_End_Device::SCHC_Session_End_Device()
{

}

uint8_t SCHC_Session_End_Device::initialize(uint8_t protocol, uint8_t direction, uint8_t dTag, SCHC_Stack_L2* stack_ptr)
{
#ifdef MYTRACE
    Serial.println("SCHC_Session_End_Device::initialize - Entering the function");
#endif
    _isUsed = false;  // at the beginning, the sessions are not being used

    if(direction==SCHC_FRAG_UP && protocol==SCHC_FRAG_LORAWAN)
    {
        // SCHC session initialisation with LoRaWAN profile parameters (see RFC9011)
        _protocol = SCHC_FRAG_LORAWAN;
        _direction = SCHC_FRAG_UP;
        _ruleID = 20;
        _dTag = dTag;
        _tileSize = 10;                         // tile size in bytes
        _m = 2;                                 // bits of the W field
        _n = 6;                                 // bits of the FCN field
        _windowSize = 63;                       // tiles in a SCHC window
        _t = 0;                                 // bits of the DTag field
        _maxAckReq = 8;                         // max number of ACK Request msg
        _retransTimer = 10;                     // Retransmission timer in seconds
        _inactivityTimer = 12*60*60;            // Inactivity timer in seconds
        _maxMsgSize = _tileSize*_windowSize*4;  // Maximum size of a SCHC packet in bytes
        _stack = stack_ptr;                     // Pointer to L2 stack
        _txAttemptsCounter = 0;                 // transmission attempt counter
    }
    else if(direction==SCHC_FRAG_DOWN && protocol==SCHC_FRAG_LORAWAN)
    {
        _protocol = SCHC_FRAG_LORAWAN;
        _direction = SCHC_FRAG_DOWN;
        _ruleID = 21;
        _dTag = dTag;
        _tileSize = 0;                          // tile size in bytes
        _m = 1;                                 // bits of the W field
        _n = 1;                                 // bits of the FCN field
        _windowSize = 1;                        // tiles in a SCHC window
        _t = 0;                                 // bits of the DTag field
        _maxAckReq = 8;
        _retransTimer = 12*60*60;               // Retransmission timer in seconds
        _inactivityTimer = 12*60*60;            // Inactivity timer in seconds
        _maxMsgSize = _tileSize*_windowSize*2;  // Maximum size of a SCHC packet in bytes
        _stack = stack_ptr;                     // Pointer to L2 stack
    }
#ifdef MYTRACE
    Serial.println("SCHC_Session_End_Device::initialize - Leaving the function");
#endif
    return 0;
}

uint8_t SCHC_Session_End_Device::startFragmentation(char *buffer, int len)
{
#ifdef MYTRACE
    Serial.println("SCHC_Session_End_Device::startFragmentation - Entering the function");
#endif
    if(_protocol==SCHC_FRAG_LORAWAN)
    {
        /*
             8.4.3.1. Sender Behavior

            At the beginning of the fragmentation of a new SCHC Packet:
            the fragment sender MUST select a RuleID and DTag value pair 
            for this SCHC Packet. A Rule MUST NOT be selected if the values 
            of M and WINDOW_SIZE for that Rule are such that the SCHC Packet 
            cannot be fragmented in (2^M) * WINDOW_SIZE tiles or less.
            the fragment sender MUST initialize the Attempts counter to 0 for 
            that RuleID and DTag value pair
        */

        if(len > (pow(2,_m)*_windowSize)*_tileSize)
        {
            char buf[100];
            sprintf(buf, "SCHC_Session_End_Device::startFragmentation - ERROR: the message is larger than %d tiles ", (int)pow(2,_m)*_windowSize);
            Serial.println(buf);
            return 1;
        }
    }

    /* Creando y inicializando maquina de estado*/
    uint8_t res = createStateMachine();
    if(res==1)
    {
        Serial.println("SCHC_Session_End_Device::startFragmentation - ERROR: Unable to create state machine");
    }

    /* Arrancando maquina de estado con el primer mensaje */
    _stateMachine->execute(buffer, len);

#ifdef MYTRACE
    Serial.println("SCHC_Session_End_Device::startFragmentation - Leaving the function");
#endif
    return 0;
}

bool SCHC_Session_End_Device::getIsUsed()
{
    return this->_isUsed;
}

void SCHC_Session_End_Device::setIsUsed(bool isUsed)
{
    this->_isUsed = isUsed;
}

void SCHC_Session_End_Device::process_message(char* msg, int len)
{
#ifdef MYTRACE
    Serial.println("SCHC_Session_End_Device::process_message - Entering the function");
#endif
    _stateMachine->queue_message(msg, len);
}

uint8_t SCHC_Session_End_Device::createStateMachine()
{
#ifdef MYTRACE
    Serial.println("SCHC_Session_End_Device::createStateMachine - Entering the function");
#endif

    if(_protocol==SCHC_FRAG_LORAWAN && _direction==SCHC_FRAG_UP)
    {
        _stateMachine = new SCHC_Ack_on_error();    // liberada en linea 146

        /* Inicializando maquina de estado */
        _stateMachine->init(_ruleID, _dTag, _windowSize, _tileSize, _n, ACK_MODE_ACK_END_WIN, _stack, _retransTimer, _maxAckReq);

#ifdef MYINFO
        Serial.println("SCHC_Session_End_Device::createStateMachine - State machine successfully created, initiated, and started");
#endif

#ifdef MYTRACE
        Serial.println("SCHC_Session_End_Device::createStateMachine - Leaving the function");
#endif
        return 0;
    }
    return 1;
}

uint8_t SCHC_Session_End_Device::destroyStateMachine()
{
    delete this->_stateMachine;
#ifdef MYINFO
        Serial.println("SCHC_Session_End_Device::createStateMachine - State machine successfully destroyed");
#endif
    return 0;
}