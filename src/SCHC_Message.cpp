#include "SCHC_Message.hpp"

SCHC_Message::SCHC_Message()
{

}

int SCHC_Message::create_regular_fragment(uint8_t ruleID, uint8_t dtag, uint8_t w, uint8_t fcn, char *payload, int payload_len, char *buffer)
{
    /* Mask definition */ 
    byte w_mask = 0xC0;
    byte fcn_mask = 0x3F;
    //byte c_mask = 0x20;

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

int SCHC_Message::create_ack_request(uint8_t ruleID, uint8_t dtag, uint8_t w, char *buffer)
{
    /* Mask definition */ 
    byte w_mask = 0xC0;
    //byte fcn_mask = 0x3F;
    //byte c_mask = 0x20;

    /* SCHC header construction */
    uint8_t new_w = (w << 6) & w_mask;
    uint8_t new_fcn = 0x00;
    uint8_t header = new_w | new_fcn;
    buffer[0] = header;

    return 1;
}

int SCHC_Message::create_sender_abort(uint8_t ruleID, uint8_t dtag, uint8_t w, char *buffer)
{
        /* Mask definition */ 
    byte w_mask = 0xC0;
    //byte fcn_mask = 0x3F;
    //byte c_mask = 0x20;

    /* SCHC header construction */
    uint8_t new_w = (w << 6) & w_mask;
    uint8_t new_fcn = 0x3F;
    uint8_t header = new_w | new_fcn;
    buffer[0] = header;

    return 1;
}

int SCHC_Message::create_all_1_fragment(uint8_t ruleID, uint8_t dtag, uint8_t w, uint32_t rcs, char *payload, int payload_len, char *buffer)
{
    /* SCHC header construction. byte 1 */
    byte w_mask = 0xC0;
    uint8_t new_w = (w << 6) & w_mask;
    uint8_t new_fcn = 0x3F;
    uint8_t header = new_w | new_fcn;
    buffer[0] = header;

    /* SCHC header construction. byte 2 al byte 5 */
    buffer[1] = (rcs >> 24) & 0xFF; // Byte más significativo
    //Serial.print((uint8_t)buffer[1], HEX);
    buffer[2] = (rcs >> 16) & 0xFF;
    //Serial.print((uint8_t)buffer[2], HEX);
    buffer[3] = (rcs >> 8) & 0xFF;
    //Serial.print((uint8_t)buffer[3], HEX);
    buffer[4] = rcs & 0xFF;        // Byte menos significativo
    //Serial.print((uint8_t)buffer[4], HEX);

    /* SCHC payload */
    for(int i=0; i<payload_len; i++)
    {
        buffer[5+i] = payload[i];
    }

    int ret = 1 + 4 + payload_len;

    return ret;
}

uint8_t SCHC_Message::get_msg_type(uint8_t protocol, int rule_id, char *msg, int len)
{
    if(protocol==SCHC_FRAG_LORAWAN)
    {
        uint8_t schc_header = msg[0];
        uint8_t c_mask = 0x20;                // Mask definition
        uint8_t _c = (c_mask & schc_header) >> 5;
        //uint8_t _dtag = 0;                      // In LoRaWAN, dtag is not used

        if(rule_id==SCHC_FRAG_UPDIR_RULE_ID && _c==1 && len==2)
            _msg_type = SCHC_RECEIVER_ABORT_MSG;
        else if(rule_id==SCHC_FRAG_UPDIR_RULE_ID && _c==1)
            _msg_type = SCHC_ACK_MSG;
        else if(rule_id==SCHC_FRAG_UPDIR_RULE_ID && _c==0)
            _msg_type = SCHC_ACK_MSG;
    }

    return _msg_type;
}

uint8_t SCHC_Message::decodeMsg(uint8_t protocol, int rule_id, char *msg, int len, uint8_t** bitmapArray)
{
    if(protocol==SCHC_FRAG_LORAWAN && rule_id == SCHC_FRAG_UPDIR_RULE_ID)
    {
        uint8_t schc_header = msg[0];
        _c = (schc_header >> 5) & 0x01;
        _w = (schc_header >> 6) & 0x03;

        //char temp[80];
        //sprintf(temp, "msg[0]: %x,_c: %d, _w: %d, len: %d",msg[0], _c,_w, len);
        //Serial.println(temp);

        if(rule_id==SCHC_FRAG_UPDIR_RULE_ID && _c==1 && len==2 && msg[1] == 0xFF)
        {
            // TODO: Se ha recibido un SCHC Receiver-Abort. No hacer nada.
        }
        else if(rule_id==SCHC_FRAG_UPDIR_RULE_ID && _c==1)
        {
            // TODO: Se ha recibido un SCHC ACK (sin errores)
            //Serial.println("SCHC_Message::decodeMsg - Receiving a SCHC ACK without errors");
            for(int i=0; i<63; i++)
            {
                bitmapArray[_w][i] = 1;
            }
        }
        else if(rule_id==SCHC_FRAG_UPDIR_RULE_ID && _c==0)
        {
            // TODO: Se ha recibido un SCHC ACK (con errores)
            //Serial.println("SCHC_Message::decodeMsg - Receiving a SCHC ACK with errors");
            int compress_bitmap_len = (len-1)*8 + 5;    // en bits
            char compress_bitmap[compress_bitmap_len];

            int k = 0;
            for(int i=4; i>=0; i--)
            {
                compress_bitmap[k] = (msg[0] >> i) & 0x01;
                k++;
            }

            for(int i=1; i<len; i++)
            {
                for(int j=7; j>=0; j--)
                {
                    compress_bitmap[k] = (msg[i] >> j) & 0x01;
                    k++;
                }
            }

            if(compress_bitmap_len >= 63)
            {
                for(int i=0; i<63; i++)
                {
                    bitmapArray[_w][i] = compress_bitmap[i];
                }
            }
            else
            {
                for(int i=0; i<compress_bitmap_len; i++)
                {
                    bitmapArray[_w][i] = compress_bitmap[i];
                }

                for(int i=compress_bitmap_len; i<63; i++)
                {
                    bitmapArray[_w][i] = 1;
                }
            }
        }  
    }

    return 0;
}

void SCHC_Message::print_msg(uint8_t msgType, char *msg, int len, uint8_t** bitmapArray)
{
    if(msgType==SCHC_REGULAR_FRAGMENT_MSG)
    {
        uint8_t schc_header = msg[0];
        uint8_t w_mask      = 0xC0;
        uint8_t fcn_mask    = 0x3F;
        uint8_t w           = (w_mask & schc_header) >> 6;
        uint8_t fcn         = fcn_mask & schc_header;
        
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

        int tile_size = 10;          // hardcoding warning - tile size = 10
        int n_tiles = (len - 1)/tile_size;   
        if(n_tiles>9)
        {
            Serial.print(n_tiles);
            Serial.print(" tiles sent");
        }
        else
        {
            Serial.print(" ");
            Serial.print(n_tiles);
            Serial.print(" tiles sent");
        }
        Serial.println();      
    }
    else if(msgType==SCHC_ACK_REQ_MSG)
    {
        uint8_t schc_header = msg[0];
        uint8_t w_mask      = 0xC0;
        uint8_t fcn_mask    = 0x3F;
        uint8_t w           = (w_mask & schc_header) >> 6;
        uint8_t fcn         = fcn_mask & schc_header;

        Serial.print("|-----W=");
        Serial.print(w);
        Serial.print(", FCN=");
        Serial.print(fcn);
        if(fcn>9)
        {
            Serial.println("----->| ");
        }
        else
        {
            Serial.println(" ----->| ");
        }
    }
    else if(msgType==SCHC_SENDER_ABORT_MSG)
    {
        uint8_t schc_header = msg[0];
        uint8_t w_mask      = 0xC0;
        uint8_t fcn_mask    = 0x3F;
        uint8_t w           = (w_mask & schc_header) >> 6;
        uint8_t fcn         = fcn_mask & schc_header;

        Serial.print("|-----W=");
        Serial.print(w);
        Serial.print(", FCN=");
        Serial.print(fcn);
        if(fcn>9)
        {
            Serial.println("----->| ");
        }
        else
        {
            Serial.println(" ----->| ");
        }
    }
    else if(msgType==SCHC_ACK_MSG)
    {
        uint8_t schc_header = msg[0];
        // Mask definition
        uint8_t w_mask = 0xC0;
        uint8_t c_mask = 0x20;
        uint8_t c = (c_mask & schc_header) >> 5;
        uint8_t w = (w_mask & schc_header) >> 6;

        Serial.print("|<----W=");
        Serial.print(w);
        Serial.print(", C=");
        Serial.print(c);
        Serial.print("---------| Bitmap: ");
        for(int i=0; i<63; i++)
        {
            Serial.print(bitmapArray[w][i]);
        }
        Serial.println();
    }
    else if(msgType==SCHC_ALL1_FRAGMENT_MSG)
    {
        uint8_t schc_header = msg[0];
        uint8_t w_mask      = 0xC0;
        uint8_t fcn_mask    = 0x3F;
        uint8_t w           = (w_mask & schc_header) >> 6;
        uint8_t fcn         = fcn_mask;

        Serial.print("|-----W=");
        Serial.print(w);
        Serial.print(", FCN=");
        Serial.print(fcn);
        if(fcn>9)
        {
            Serial.print("+RCS->| last tile: ");
            Serial.print((len-5)*8);
            Serial.println(" bits");

        }
        else
        {
            Serial.print(" +RCS->| last tile: ");
            Serial.print((len-5)*8);
            Serial.println(" bits");

        }

    }
    else if(msgType==SCHC_RECEIVER_ABORT_MSG)
    {
        Serial.print("|<--SCHC Recv-Abort ---|");
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

uint8_t SCHC_Message::get_w()
{
    return _w;
}

uint8_t SCHC_Message::get_c()
{
    return _c;
}
