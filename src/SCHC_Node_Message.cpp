#include "SCHC_Node_Message.hpp"

SCHC_Node_Message::SCHC_Node_Message()
{

}

int SCHC_Node_Message::create_regular_fragment(uint8_t ruleID, uint8_t dtag, uint8_t w, uint8_t fcn, char *payload, int payload_len, char *buffer)
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

int SCHC_Node_Message::create_ack_request(uint8_t ruleID, uint8_t dtag, uint8_t w, char *buffer)
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

int SCHC_Node_Message::create_sender_abort(uint8_t ruleID, uint8_t dtag, uint8_t w, char *buffer)
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

int SCHC_Node_Message::create_all_1_fragment(uint8_t ruleID, uint8_t dtag, uint8_t w, uint32_t rcs, char *payload, int payload_len, char *buffer)
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

uint8_t SCHC_Node_Message::get_msg_type(uint8_t protocol, int rule_id, char *msg, int len)
{
    if(protocol==SCHC_FRAG_LORAWAN)
    {
        uint8_t schc_header = msg[0];
        uint8_t _c = (schc_header >> 5) & 0x01;

        if(rule_id==SCHC_FRAG_UPDIR_RULE_ID && _c==1 && len==2)
            _msg_type = SCHC_RECEIVER_ABORT_MSG;
        else if(rule_id==SCHC_FRAG_UPDIR_RULE_ID && _c==1)
            _msg_type = SCHC_ACK_MSG;
        else if(rule_id==SCHC_FRAG_UPDIR_RULE_ID && _c==0 && len>9)
            _msg_type = SCHC_COMPOUND_ACK;
        else if(rule_id==SCHC_FRAG_UPDIR_RULE_ID && _c==0)     
            _msg_type = SCHC_ACK_MSG;
    }

    return _msg_type;
}

uint8_t SCHC_Node_Message::decodeMsg(uint8_t protocol, int rule_id, char *msg, int len, uint8_t** bitmapArray)
{
    if(protocol==SCHC_FRAG_LORAWAN && rule_id == SCHC_FRAG_UPDIR_RULE_ID)
    {
        uint8_t schc_header = msg[0];
        _c = (schc_header >> 5) & 0x01;
        _w = (schc_header >> 6) & 0x03;

        if(rule_id==SCHC_FRAG_UPDIR_RULE_ID && _c==1 && len==2 && msg[1] == 0xFF)
        {
            // TODO: Se ha recibido un SCHC Receiver-Abort. No hacer nada.
        }
        else if(rule_id==SCHC_FRAG_UPDIR_RULE_ID && _c==1)
        {
            // * Se ha recibido un SCHC ACK (sin errores)
            for(int i=0; i<63; i++)
            {
                bitmapArray[_w][i] = 1;
            }
        }
        else if(rule_id==SCHC_FRAG_UPDIR_RULE_ID && _c==0 && len>9)
        {
            // * Se ha recibido un SCHC Compound ACK (con errores)
            //Serial.println("SCHC_Node_Message::decodeMsg - Receiving a SCHC Compound ACK with errors");
            int n_total_bits    = len*8;                    // en bits
            int n_win           = ceil((n_total_bits - 1)/65);     // window_size + M = 65. Se resta un bit a len debido al bit C.
            //int n_padding_bits  = n_total_bits - 1 - n_win*65;
            bool first_win      = true;

            std::vector<uint8_t> bitVector;
            
            for (int i = 0; i < len; ++i)
            {
                for (int j = 7; j >= 0; --j)
                {
                    bitVector.push_back((msg[i] >> j) & 1);
                }
            }

            for(int i=0; i<n_win; i++)
            {
                if(first_win)
                {
                    _windows_with_error.push_back(_w);      // almacena en el vector el numero de la primera ventana con error en el SCHC Compound ACK
                    
                    bitVector.erase(bitVector.begin(), bitVector.begin()+3); // Se elimina del vector la ventana (2 bits) y c (1 bit)
                    std::copy(bitVector.begin(), bitVector.begin() + 63, bitmapArray[_w]);
                    bitVector.erase(bitVector.begin(), bitVector.begin()+63);
                    first_win = false;
                }
                else
                {
                    uint8_t win = (bitVector[0] << 1) | bitVector[1];
                    _windows_with_error.push_back(win);
                    bitVector.erase(bitVector.begin(), bitVector.begin()+2);
                    std::copy(bitVector.begin(), bitVector.begin() + 63, bitmapArray[win]);
                    bitVector.erase(bitVector.begin(), bitVector.begin()+63);
                }
            }

        }
        else if(rule_id==SCHC_FRAG_UPDIR_RULE_ID && _c==0)
        {
            if(_msg_type == ACK_MODE_ACK_END_WIN || _msg_type == ACK_MODE_ACK_END_SES)
            {    
                // * Se ha recibido un SCHC ACK (con errores)
                //Serial.println("SCHC_Node_Message::decodeMsg - Receiving a SCHC ACK with errors");
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
            else if(_msg_type == ACK_MODE_COMPOUND_ACK)
            {
                // * Se ha recibido un SCHC Compound ACK (con errores)
                //Serial.println("SCHC_Node_Message::decodeMsg - Receiving a SCHC Compound ACK with errors");

                int n_total_bits    = len*8;                        // en bits
                int n_win           = ceil((n_total_bits - 1)/65);  // window_size + M = 65. Se resta un bit a len debido al bit C.
                //int n_padding_bits  = n_total_bits - 1 - n_win*65;
                bool first_win      = true;

                std::vector<uint8_t> bitVector;
                
                /* traspasa el mensaje de formato char a vector*/
                for (int i = 0; i < len; ++i)
                {
                    for (int j = 7; j >= 0; --j)
                    {
                        bitVector.push_back((msg[i] >> j) & 1);
                    }
                }

                for(int i=0; i<n_win; i++)
                {
                    if(first_win)
                    {
                        _windows_with_error.push_back(_w);      // almacena en el vector el numero de la primera ventana con error en el SCHC Compound ACK
                        
                        bitVector.erase(bitVector.begin(), bitVector.begin()+3); // Se elimina del vector la ventana (2 bits) y c (1 bit)
                        std::copy(bitVector.begin(), bitVector.begin() + 63, bitmapArray[_w]);
                        bitVector.erase(bitVector.begin(), bitVector.begin()+63);
                        first_win = false;
                    }
                    else
                    {
                        uint8_t win = (bitVector[0] << 1) | bitVector[1];
                        _windows_with_error.push_back(win);
                        bitVector.erase(bitVector.begin(), bitVector.begin()+2);
                        std::copy(bitVector.begin(), bitVector.begin() + 63, bitmapArray[win]);
                        bitVector.erase(bitVector.begin(), bitVector.begin()+63);
                    }
                }

            }
        }
          
    }

    return 0;
}

void SCHC_Node_Message::print_msg(uint8_t msgType, char *msg, int len, uint8_t** bitmapArray)
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

        if(c == 1)
        {
            Serial.print("|<----W=");
            Serial.print(w);
            Serial.print(", C=");
            Serial.print(c);
            Serial.print(" --------| C=1");
            Serial.println();
        }
        else
        {
            Serial.print("|<----W=");
            Serial.print(w);
            Serial.print(", C=");
            Serial.print(c);
            Serial.print(" --------| Bitmap: ");
            for(int i=0; i<63; i++)
            {
                Serial.print(bitmapArray[w][i]);
            }
            Serial.println();
        }

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
    else if(msgType==SCHC_COMPOUND_ACK)
    {
        uint8_t schc_header = msg[0];
        uint8_t c_mask      = 0x20;
        uint8_t c           = (c_mask & schc_header) >> 5;

        Serial.print("|<--- ACK, C=");
        Serial.print(c);
        Serial.print(" --------|");

        for(uint8_t i=0; i<_windows_with_error.size(); i++)
        {
            uint8_t win = _windows_with_error[i];
            Serial.print(", W=");
            Serial.print(win);
            Serial.print(" - Bitmap:");
            for(int i=0; i<63; i++)
            {
                Serial.print(bitmapArray[win][i]);
            }
        }
        Serial.println();
    }
    else if(msgType==SCHC_ACK_RESIDUAL_MSG)
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
        Serial.print(" --------| Residual ACK, dropped");
        Serial.println();        
    }

}

void SCHC_Node_Message::printBin(uint8_t val)
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

uint8_t SCHC_Node_Message::get_w()
{
    return _w;
}

std::vector<uint8_t> SCHC_Node_Message::get_w_vector()
{
    return _windows_with_error;
}

uint8_t SCHC_Node_Message::get_c()
{
    return _c;
}
