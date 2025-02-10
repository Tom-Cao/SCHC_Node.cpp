#include "SCHC_Node_Fragmenter.hpp"


SCHC_Node_Fragmenter::SCHC_Node_Fragmenter()
{
    
}

uint8_t SCHC_Node_Fragmenter::initialize(uint8_t protocol)
{
#ifdef MYTRACE
    Serial.println("SCHC_Node_Fragmenter::initialize - Entering the function");
#endif
    _protocol = protocol;
    if(protocol==SCHC_FRAG_LORAWAN)
    {
#ifdef MYINFO
        Serial.println("SCHC_Node_Fragmenter::initialize - Initializing LoRaWAN stack");
#endif 
        _stack = new SCHC_Node_LoRaWAN_RAK4631();
        SCHC_Node_LoRaWAN_RAK4631* lora_stack = static_cast<SCHC_Node_LoRaWAN_RAK4631*>(_stack);
        lora_stack->initialize_stack();
        lora_stack->set_fragmenter(this);

        /* initializing the session pool */
#ifdef MYINFO
        Serial.println("SCHC_Node_Fragmenter::initialize - Initializing session pool");
#endif 
        for(uint8_t i=0; i<_SESSION_POOL_SIZE; i++)
        {
            _uplinkSessionPool[i].initialize(SCHC_FRAG_LORAWAN,
                                            SCHC_FRAG_UP,
                                            i,
                                            _stack);
            _downlinkSessionPool[i].initialize(SCHC_FRAG_LORAWAN,
                                            SCHC_FRAG_DOWN,
                                            i,
                                            _stack);
        }

        /* initializing the LoRaWAN Stack*/
#ifdef MYINFO
        Serial.println("SCHC_Node_Fragmenter::initialize - Instantiation and Initializing LoRaWAN Stack");
#endif
    }

#ifdef MYTRACE
    Serial.println("SCHC_Node_Fragmenter::initialize - Leaving the function");
#endif
    return 0;
}

uint8_t SCHC_Node_Fragmenter::send(char* buffer, int len)
{
#ifdef MYTRACE
    Serial.println("SCHC_Node_Fragmenter::send - Entering the function");
#endif

    int id = get_free_session_id(SCHC_FRAG_UP);
    if(id != -1)
    {
        SCHC_Node_Session us = _uplinkSessionPool[id];
        this->associate_session_id(SCHC_FRAG_UPDIR_RULE_ID, id);
        us.startFragmentation(buffer, len);
        us.setIsUsed(false);
    }

#ifdef MYTRACE
    Serial.println("SCHC_Node_Fragmenter::send - Leaving the function");
#endif
    return 0;
}

uint8_t SCHC_Node_Fragmenter::process_received_message(char* buffer, int len, int fport)
{
#ifdef MYTRACE
    Serial.println("SCHC_Node_Fragmenter::listen_message - Entering the function");
#endif

    // Valida si existe una sesión asociada al deviceId.
    int id = this->get_session_id(fport);
    if(id == -1)
    {
        //TBD Downlink fragmentation
    }
    else
    {
#ifdef MYTRACE
        Serial.println("SCHC_Node_Fragmenter::process_received_message - Obtaining session id");
#endif
        if(fport == SCHC_FRAG_UPDIR_RULE_ID)
        {
            _uplinkSessionPool[id].process_message(buffer, len);
        }
    }


#ifdef MYTRACE
        Serial.println("SCHC_Node_Fragmenter::listen_message - Leaving the function");
#endif
    return 0;
}

int SCHC_Node_Fragmenter::get_free_session_id(uint8_t direction)
{
#ifdef MYTRACE
    Serial.println("SCHC_Node_Fragmenter::getSessionId - Entering the function");
#endif
    if(_protocol==SCHC_FRAG_LORAWAN && direction==SCHC_FRAG_UP)
    {
        for(uint8_t i=0; i<_SESSION_POOL_SIZE;i++)
        {
            if(!_uplinkSessionPool[i].getIsUsed())
            {
#ifdef MYTRACE
    Serial.println("SCHC_Node_Fragmenter::getSessionId - Leaving the function");
#endif
                return i;
            }
        }
        Serial.println("SCHC_Node_Fragmenter::getSessionId - ERROR: all sessiones are used");
    }
#ifdef MYTRACE
    Serial.println("SCHC_Node_Fragmenter::getSessionId - Leaving the function");
#endif
    return -1;
}

uint8_t SCHC_Node_Fragmenter::associate_session_id(int rule_id, int sessionId)
{
        auto result = _associationMap.insert({rule_id, sessionId});
        if (result.second)
        {
#ifdef MYTRACE
                Serial.println("SCHC_Node_Fragmenter::associate_session_id - Key and value successfully inserted in the map.");
#endif
                return 0;
        } else
        {
#ifdef MYDEBUG
                Serial.println("The key already exists in the map. Key");
#endif
                return -1;
        }    
    return 0;
}

uint8_t SCHC_Node_Fragmenter::disassociate_session_id(int rule_id)
{
        size_t res = _associationMap.erase(rule_id);
        if(res == 0)
        {
#ifdef MYDEBUG
                Serial.println("Key not found. Could not disassociate.");
#endif
                return -1;
        }
        else if(res == 1)
        {
#ifdef MYDEBUG
                Serial.println("Key successfully disassociated.");
#endif
                return 0;
        }
        return -1;
}

int SCHC_Node_Fragmenter::get_session_id(int rule_id)
{
        auto it = _associationMap.find(rule_id);
        if (it != _associationMap.end())
        {
#ifdef MYTRACE
                Serial.println("SCHC_Node_Fragmenter::get_session_id - Recovering the session id from the map.");
#endif
                return it->second;
        }
        else
        {
#ifdef MYDEBUG
                Serial.println("Session does not exist.");
#endif
                return -1;
        }
}
