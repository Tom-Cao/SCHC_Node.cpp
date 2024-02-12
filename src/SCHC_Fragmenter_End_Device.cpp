#include "SCHC_Fragmenter_End_Device.hpp"


SCHC_Fragmenter_End_Device::SCHC_Fragmenter_End_Device()
{
    
}

uint8_t SCHC_Fragmenter_End_Device::initialize(uint8_t protocol)
{
#ifndef MYDEBUG
    Serial.println("SCHC_Fragmenter_End_Device::initialize - Entering the function");
#endif
    _protocol = protocol;
    if(protocol==SCHC_FRAG_PROTOCOL_LORAWAN)
    {
#ifndef MYINFO
        Serial.println("SCHC_Fragmenter_End_Device::initialize - Initializing LoRaWAN stack");
#endif 
        _stack = new LoRaWAN_RAK4631();
        _stack->initialize_stack();

        /* initializing the session pool */
#ifndef MYINFO
        Serial.println("SCHC_Fragmenter_End_Device::initialize - Initializing session pool");
#endif 
        for(uint8_t i=0; i<_SESSION_POOL_SIZE; i++)
        {
            _uplinkSessionPool[i].initialize(SCHC_FRAG_PROTOCOL_LORAWAN,
                                            SCHC_FRAG_DIRECTION_UPLINK,
                                            i,
                                            _stack);
            _downlinkSessionPool[i].initialize(SCHC_FRAG_PROTOCOL_LORAWAN,
                                            SCHC_FRAG_DIRECTION_DOWNLINK,
                                            i,
                                            _stack);
        }

        /* initializing the LoRaWAN Stack*/
#ifndef MYINFO
        Serial.println("SCHC_Fragmenter_End_Device::initialize - Instantiation and Initializing LoRaWAN Stack");
#endif


    }

#ifndef MYDEBUG
    Serial.println("SCHC_Fragmenter_End_Device::initialize - Leaving the function");
#endif
    return 0;
}

uint8_t SCHC_Fragmenter_End_Device::send(char *buffer, int len)
{
#ifndef MYDEBUG
    Serial.println("SCHC_Fragmenter_End_Device::send - Entering the function");
#endif

    int id = getSessionId(SCHC_FRAG_DIRECTION_UPLINK);
    if(id != -1)
    {
        SCHC_Session_End_Device us = _uplinkSessionPool[id];
        us.startFragmentation(buffer, len);
    }

#ifndef MYDEBUG
    Serial.println("SCHC_Fragmenter_End_Device::send - Leaving the function");
#endif
    return 0;
}

int SCHC_Fragmenter_End_Device::getSessionId(uint8_t direction)
{
#ifndef MYDEBUG
    Serial.println("SCHC_Fragmenter_End_Device::getSessionId - Entering the function");
#endif
    if(_protocol==SCHC_FRAG_PROTOCOL_LORAWAN && direction==SCHC_FRAG_DIRECTION_UPLINK)
    {
        for(uint8_t i=0; i<_SESSION_POOL_SIZE;i++)
        {
            if(!_uplinkSessionPool[i].isUsed())
            {
#ifndef MYDEBUG
    Serial.println("SCHC_Fragmenter_End_Device::getSessionId - Leaving the function");
#endif
                return i;
            }
        }
        Serial.println("SCHC_Fragmenter_End_Device::getSessionId - ERROR: all sessiones are used");
    }
#ifndef MYDEBUG
    Serial.println("SCHC_Fragmenter_End_Device::getSessionId - Leaving the function");
#endif
    return -1;
}
