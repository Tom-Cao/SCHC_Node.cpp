#include "SCHC_Message.hpp"

SCHC_Message::SCHC_Message()
{

}

int SCHC_Message::createRegularFragment(uint8_t ruleID, uint8_t dtag, uint8_t w, uint8_t fcn, char *payload, int payload_len, char *buffer)
{
    /* Mask definition */ 
    byte w_mask = 0xC0;
    byte fcn_mask = 0x3F;
    byte c_mask = 0x20;

    /* SCHC header construction */
    uint8_t new_w = (w << 6) & w_mask;
    uint8_t new_fcn = (fcn & fcn_mask);
    uint8_t header = new_w | new_fcn;
    buffer[0] = header;

    /* SCHC payload construction */
    for(int i=0;i<payload_len;i++)
    {
        buffer[i+1] = payload[i];
    }

    return (payload_len + 1);
}

SCHC_Message *SCHC_Message::decodeMsg(uint8_t protocol, char *msg, int len)
{
    if(protocol==SCHC_FRAG_PROTOCOL_LORAWAN)
    {
        byte schc_header = (byte)msg[0];

        // Mask definition
        byte w_mask = 0xC0;
        byte fcn_mask = 0x3F;
        byte c_mask = 0x20;

        uint8_t w = w_mask && schc_header;
        
    }

    return this;
}

void SCHC_Message::printMsg(uint8_t protocol, uint8_t msgType, char *msg, int len)
{
    // if(protocol==SCHC_FRAG_PROTOCOL_LORAWAN)
    // {
    //     Serial.print("SCHC Header ---> ");
    //     printBin((uint8_t)msg[0]);
    //     Serial.println();

    //     Serial.print("SCHC Payload --> ");
    //     for(int i=1; i<len; i++)
    //     {
    //         Serial.print(msg[i]);
    //     }
    //     Serial.println();
    // }

    // |-----W=0, FCN=27----->| 4 tiles sent

    if(msgType==SCHC_REGULAR_FRAGMENT_MSG)
    {
        uint8_t w = (msg[0] & 0xC0) >> 6;
        uint8_t fcn = (msg[0] & 0x3F);
        Serial.print("|-----W=");
        Serial.print(w);
        Serial.print(", FCN=");
        Serial.print(fcn);
        if(fcn>9)
        {
            Serial.print("----->| ");
        }
        else
        {
            Serial.print(" ----->| ");
        }
        
        Serial.print(len/10);
        Serial.print(" tiles sent");
    }


}

void SCHC_Message::printBin(uint8_t val)
{
    if(val<2)
    {
        Serial.print("0000000");
        Serial.print(val,BIN);
    }
    else if (val<4)
    {
        Serial.print("000000");
        Serial.print(val,BIN);
    }
    else if (val<8)
    {
        Serial.print("00000");
        Serial.print(val,BIN);
    }
    else if (val<16)
    {
        Serial.print("0000");
        Serial.print(val,BIN);
    }
    else if (val<32)
    {
        Serial.print("000");
        Serial.print(val,BIN);
    }
    else if (val<64)
    {
        Serial.print("00");
        Serial.print(val,BIN);
    }
    else if (val<128)
    {
        Serial.print("0");
        Serial.print(val,BIN);
    }
    else if (val<256)
    {
        Serial.print(val,BIN);
    }
    
}
