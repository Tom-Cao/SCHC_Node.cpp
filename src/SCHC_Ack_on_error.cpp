#include "SCHC_Ack_on_error.hpp"

SCHC_Ack_on_error::SCHC_Ack_on_error()
{
}

SCHC_Ack_on_error::~SCHC_Ack_on_error()
{
}

uint8_t SCHC_Ack_on_error::init(uint8_t ruleID, uint8_t dTag, uint8_t windowSize, uint8_t tileSize, uint8_t n, uint8_t ackMode, SCHC_Stack_L2 *stack_ptr, int retTimer, uint8_t ackReqAttempts)
{
#ifdef MYTRACE
    Serial.println("SCHC_Ack_on_error::init - Entering the function");
#endif
    /* Static SCHC parameters */
    _currentState = STATE_TX_INIT;
    _ruleID = ruleID;
    _dTag = dTag;
    _windowSize = windowSize;
    _nFullTiles = 0;                // in tiles
    _lastTileSize = 0;              // in bytes
    _tileSize = tileSize;           // in bytes
    _ackMode = ackMode;
    _retransTimer = retTimer;       // in millis
    _maxAckReq = ackReqAttempts;

    /* Static LoRaWAN parameters*/
    _current_L2_MTU = stack_ptr->getMtu(true);
    _stack = stack_ptr;

    _queue = xQueueCreate(10, sizeof(Data_packet_t));
    _running = true;

#ifdef MYTRACE
    Serial.println("SCHC_Ack_on_error::init - Leaving the function");
#endif

    return 0;
}

uint8_t SCHC_Ack_on_error::execute(char *msg, int len)
{
#ifdef MYTRACE
    Serial.println("SCHC_Ack_on_error::execute - Entering the function");
#endif
    if(msg!=NULL)
    {
        if(_currentState==STATE_TX_INIT)
        {
            this->TX_INIT_send_fragments(msg, len);

        }
        else if(_currentState==STATE_TX_WAIT_x_ACK)
        {
            this->TX_WAIT_x_ACK_receive_ack(msg, len);
        }
    }
    else
    {
        if(_currentState==STATE_TX_SEND)
        {
            this->TX_SEND_send_fragments();
        }
        else if(_currentState==STATE_TX_END)
        {
            this->TX_END_free_resources();
        }
    }
#ifdef MYTRACE
    Serial.println("SCHC_Ack_on_error::execute - Leaving the function");
#endif
    return 0;
}

uint8_t SCHC_Ack_on_error::queue_message(char *msg, int len)
{
    Data_packet_t packet;
    memcpy(packet.msg, msg, len);
    packet.len = len;

    if (xQueueSend(_queue, &packet, portMAX_DELAY) == pdPASS)
    {
#ifdef MYTRACE
        Serial.println("SCHC_Ack_on_error::queue_message - Data successfully queue");
#endif
      
    }
    return 0;
}

void SCHC_Ack_on_error::message_reception_loop()
{
    while (_running)
    {
#ifdef MYTRACE
        Serial.println("SCHC_Ack_on_error::message_reception_loop - Extracting message from the queue.");
#endif
        Data_packet_t receivedPacket;

        if (xQueueReceive(_queue, &receivedPacket, portMAX_DELAY) == pdPASS)
        {
            this->execute(receivedPacket.msg, receivedPacket.len);

            vTaskDelay(pdMS_TO_TICKS(1000)); // Pausar por 1000 ms
        }

    }
    
}

uint8_t SCHC_Ack_on_error::TX_INIT_send_fragments(char *msg, int len)
{
#ifdef MYTRACE
    Serial.println("SCHC_Ack_on_error::TX_INIT_send_fragments - Entering the function");
#endif

    /* Dynamic SCHC parameters */
    _currentWindow = 0;
    _currentFcn = (_windowSize)-1;
    _currentBitmap_ptr = 0;
    _currentTile_ptr = 0;

    uint8_t res;
    res = this->divideInTiles(msg, len);
    if(res==1)
    {
        Serial.println("SCHC_Ack_on_error::TX_INIT_send_fragments - ERROR when splitting the message into tiles");
    }

    _currentState = STATE_TX_SEND;
#ifdef MYDEBUG
    Serial.println("Changing STATE: From STATE_TX_INIT --> STATE_TX_SEND");
#endif

    res = this->execute();

#ifdef MYTRACE
    Serial.println("SCHC_Ack_on_error::TX_INIT_send_fragments - Leaving the function");
#endif
    return res;
}

uint8_t SCHC_Ack_on_error::TX_SEND_send_fragments()
{
#ifdef MYTRACE
    Serial.println("SCHC_Ack_on_error::TX_SEND_send_fragment - Entering the function");
#endif

    //printTileArray();

    /* Numero de tiles que se pueden enviar un un payload */
    int payload_available_in_bytes = _current_L2_MTU - 1; // MTU = SCHC header + SCHC payload
    int payload_available_in_tiles = payload_available_in_bytes/_tileSize;

    /* Variables temporales */
    int n_tiles_to_send = 0;        // numero de tiles a enviar
    int n_remaining_win_tiles = 0;  // n de tiles restantes por enviar (usado en el modo de confirmacion por ventana) 
    int n_remaining_tot_tiles = 0;  // n de full tiles totales que faltan por enviar
    int n_all_remaining_tiles = 0;  // n de tiles restantes por enviar (usado en el modo de confirmacion por sesion)
    int n_remaining_tiles     = 0;  // n de tiles que faltan por enviar. Su valor depende de si los tiles totales por enviar es mayor o menor a los tiles faltantes de enviar de la ventana
    
    SCHC_Message encoder;           // encoder 

    if(_ackMode==ACK_MODE_ACK_END_WIN)
    {
        n_remaining_win_tiles = _currentFcn + 1;                // numero de tiles de la ventana que faltan por enviar
        n_remaining_tot_tiles = _nFullTiles - _currentTile_ptr; // numero total de tiles que faltan por enviar

        if(n_remaining_tot_tiles <= n_remaining_win_tiles)
            n_remaining_tiles = n_remaining_tot_tiles;
        else
            n_remaining_tiles = n_remaining_win_tiles;

        if(n_remaining_tiles>payload_available_in_tiles)
        {
            /* Si la cantidad de tiles que faltan por enviar 
            para la ventana i-esima no alcanza en la MTU 
            significa que aun NO finaliza la ventana i-esima
            */
            n_tiles_to_send = payload_available_in_tiles;

            /* buffer que almacena todos los tiles que se van a enviar */
            int payload_len         = n_tiles_to_send * _tileSize;  // tamaño del SCHC payload en bytes
            char* schc_payload      = new char[payload_len];        // liberado en linea 215. buffer para el SCHC payload
            char* schc_message      = new char[payload_len + 1];    // liberado en linea 214 buffer para el SCHC message (header + payload)            
            
            this->extractTiles(_currentTile_ptr, n_tiles_to_send, schc_payload);

            /* Crea un mensaje SCHC en formato hexadecimal */
            int schc_message_len = encoder.create_regular_fragment(_ruleID, _dTag, _currentWindow, _currentFcn, schc_payload, payload_len, schc_message);

            /* Imprime los mensajes para visualizacion ordenada */
            encoder.print_msg(SCHC_REGULAR_FRAGMENT_MSG, schc_message, schc_message_len);

            /* Envía el mensaje a la capa 2*/
            uint8_t res = _stack->send_frame(_ruleID, schc_message, schc_message_len);
            if(res==1)
            {
                Serial.println("SCHC_Ack_on_error::sendRegularFragment - ERROR sending L2 frame");
                return 1;
            }

            /* Eliminar los punteros a buffers*/
            delete[] schc_message;
            delete[] schc_payload;

            
            _currentTile_ptr    = _currentTile_ptr + n_tiles_to_send;
            _currentFcn         = _currentFcn - n_tiles_to_send;
            _currentState       = STATE_TX_SEND;

            this->execute();
        }
        else
        {
            /* Si la cantidad de tiles que faltan por enviar 
            para la ventana i-esima SI alcanza en la MTU 
            significa que este es el ultimo envio para la 
            ventana i-esima
            */
            n_tiles_to_send = n_remaining_tiles;

            /* buffer que almacena todos los tiles que se van a enviar */
            int payload_len         = n_tiles_to_send * _tileSize;  // tamaño del SCHC payload en bytes
            char* schc_payload      = new char[payload_len];        // liberado en linea 258. buffer para el SCHC payload
            char* schc_message      = new char[payload_len + 1];    // liberado en linea 257. buffer para el SCHC message (header + payload)

            this->extractTiles(_currentTile_ptr, n_tiles_to_send, schc_payload);

            /* Crea un mensaje SCHC en formato hexadecimal */
            int schc_message_len = encoder.create_regular_fragment(_ruleID, _dTag, _currentWindow, _currentFcn, schc_payload, payload_len, schc_message);

            /* Imprime los mensajes para visualizacion ordenada */
            encoder.print_msg(SCHC_REGULAR_FRAGMENT_MSG, schc_message, schc_message_len); 

            /* Envía el mensaje a la capa 2*/
            uint8_t res = _stack->send_frame(_ruleID, schc_message, schc_message_len);
            if(res==1)
            {
                Serial.println("SCHC_Ack_on_error::sendRegularFragment - ERROR sending L2 frame");
                return 1;
            }

            /* Eliminar los punteros a buffers*/
            delete[] schc_message;
            delete[] schc_payload;

            _currentTile_ptr    = _currentTile_ptr + n_tiles_to_send;
            _currentState       = STATE_TX_WAIT_x_ACK;


            /* Se envia el ultimo tile en un SCHC All-1 */
            if(_currentWindow == (_nWindows - 1))
            {
                /* buffer que almacena todos los tiles que se van a enviar */
                payload_len                 = _lastTileSize;                    // tamaño del last tile en bytes
                char* schc_all_1_message    = new char[payload_len + 1 + 4];    // liberado en linea 285. buffer para el SCHC message: header (1B) + rcs (4B) + payload
                /* Crea un mensaje SCHC en formato hexadecimal */
                int schc_all_1_message_len = encoder.create_all_1_fragment(_ruleID, _dTag, _currentWindow, _rcs, _lastTile, payload_len, schc_all_1_message);

                /* Imprime los mensajes para visualizacion ordenada */
                encoder.print_msg(SCHC_ALL1_FRAGMENT_MSG, schc_all_1_message, schc_all_1_message_len); 

                /* Envía el mensaje a la capa 2*/
                uint8_t res = _stack->send_frame(_ruleID, schc_all_1_message, schc_all_1_message_len);
                if(res==1)
                {
                    Serial.println("SCHC_Ack_on_error::sendRegularFragment - ERROR sending L2 frame");
                    return 1;
                }

                /* Eliminar los punteros a buffers*/
                delete[] schc_all_1_message;
            }

            /* Se envía un SCHC ACK REQ para empujar el envio en el downlink
            del SCHC ACK enviado por el SCHC Gateway */
            vTaskDelay(pdMS_TO_TICKS(1000)); // Pausar por 1000 ms

            SCHC_Message encoder_2;
            char* schc_ack_req_msg      = new char[1];      // liberado en linea 308
            int schc_ack_req_msg_len    = encoder_2.create_ack_request(_ruleID, _dTag, _currentWindow, schc_ack_req_msg);

            /* Imprime los mensajes para visualizacion ordenada */
            encoder_2.print_msg(SCHC_ACK_REQ_MSG, schc_ack_req_msg, schc_ack_req_msg_len);

            /* Envía el mensaje a la capa 2*/
            res = _stack->send_frame(_ruleID, schc_ack_req_msg, schc_ack_req_msg_len);
            if(res==1)
            {
                Serial.println("SCHC_Ack_on_error::TX_WAIT_x_ACK_send_ack_req - ERROR sending L2 frame");
                return 1;
            }

            /* Eliminar los punteros a buffers*/
            delete[] schc_ack_req_msg;

#ifdef MYDEBUG
            Serial.println("Changing STATE: From STATE_TX_SEND --> STATE_TX_WAIT_x_ACK");
#endif
            this->message_reception_loop();

        }
    }
    else if(_ackMode==ACK_MODE_ACK_END_SES || _ackMode==ACK_MODE_COMPOUND_ACK)
    {
        n_all_remaining_tiles = _nFullTiles - _currentTile_ptr; // numero de tiles de toda la sesion que faltan por enviar
        if(n_all_remaining_tiles>payload_available_in_tiles)
        {
            /* Si la cantidad de tiles que faltan por enviar 
            para la sesion no alcanza en la MTU significa 
            que aun NO finaliza la sesion
            */
            n_tiles_to_send = payload_available_in_tiles;

            // newBitmap_ptr = this->setBitmapArray(_currentTile_ptr, n_tiles_to_send);
            // newFcn = this->getCurrentFCN(_currentTile_ptr + n_tiles_to_send);
            // newWindow = this->getCurrentWindow(_currentTile_ptr + n_tiles_to_send);
            // _currentState = STATE_TX_SEND;
        }
        else
        {
            /* Si la cantidad de tiles que faltan por enviar 
            para la sesion SI alcanza en la MTU 
            significa que estos son los ultimos Full Tiles
            */
            n_tiles_to_send = n_all_remaining_tiles;

            // newBitmap_ptr = this->setBitmapArray(_currentTile_ptr, n_tiles_to_send);
            // newFcn = _windowSize;
            // newWindow = this->getCurrentWindow(_currentTile_ptr + n_tiles_to_send);
            // _currentState = STATE_TX_WAIT_x_ACK;
        }        
    }

#ifdef MYTRACE
    Serial.println("SCHC_Ack_on_error::TX_SEND_send_fragment - Leaving the function");
#endif

    return 0;
}

uint8_t SCHC_Ack_on_error::TX_WAIT_x_ACK_receive_ack(char *msg, int len)
{
#ifdef MYTRACE
    Serial.println("SCHC_Ack_on_error::TX_WAIT_x_ACK_send_ack_req - Entering the function");
#endif
    
    SCHC_Message decoder;
    uint8_t msg_type = decoder.get_msg_type(SCHC_FRAG_LORAWAN, SCHC_FRAG_UPDIR_RULE_ID, msg, len);

    if(msg_type == SCHC_ACK_MSG)
    {
        decoder.decodeMsg(SCHC_FRAG_LORAWAN, SCHC_FRAG_UPDIR_RULE_ID, msg, len);
        uint8_t c = decoder.get_c();
        uint8_t w = decoder.get_w();
        if(c == 1 && w == (_nWindows-1))
        {
            /* SCHC ACK incluye bitmap sin errores y es un ACK a la ultima ventana*/

            decoder.print_msg(SCHC_ACK_MSG, msg, len);

            _currentFcn         = (_windowSize)-1;
            _currentWindow      = _currentWindow + 1; 
            _currentState = STATE_TX_END;
            
#ifdef MYDEBUG
            Serial.println("Changing STATE: From STATE_TX_WAIT_x_ACK --> STATE_TX_END");
#endif
            this->execute();
        }
        else if(c == 1 && w != (_nWindows-1))
        {
            /* SCHC ACK incluye bitmap sin errores y NO es un ACK para la ultima ventana*/
            decoder.print_msg(SCHC_ACK_MSG, msg, len);

            _currentFcn         = (_windowSize)-1;
            _currentWindow      = _currentWindow + 1; 
            _currentState = STATE_TX_SEND;
            
#ifdef MYDEBUG
            Serial.println("Changing STATE: From STATE_TX_WAIT_x_ACK --> STATE_TX_SEND");
#endif
            this->execute();            

        }
        else if(c == 0)
        {
            // TODO: SCHC ACK incluye bitmap con errores
        }

    }
    else if(msg_type == SCHC_RECEIVER_ABORT_MSG)
    {
#ifdef MYDEBUG
            Serial.println("Receiving a SCHC Receiver-Abort message. Releasing resources...");
#endif
            // TODO: Liberar maquinas de estados y recursos de memoria. Se debe retornar la funcion SCHC_Fragmenter_End_Device::send()
    }


#ifdef MYTRACE
    Serial.println("SCHC_Ack_on_error::TX_WAIT_x_ACK_send_ack_req - Leaving the function");
#endif 
    return 0;
}

uint8_t SCHC_Ack_on_error::TX_END_free_resources()
{
    // TODO: Liberar recursos, liberar maquina, liberar sesion
#ifdef MYINFO
    Serial.println("SCHC_Ack_on_error::TX_END_free_resources - Releasing resources and finish machines and session");
#endif

 
    for(int i = 0 ; i < _nFullTiles ; i++ )
    {
        delete[] _tilesArray[i];
    }
    delete[] _tilesArray;
    delete[] _lastTile;

    _running = false;

    return 0;
}

uint8_t SCHC_Ack_on_error::mtuUpgrade(int mtu)
{
#ifdef MYTRACE
    Serial.println("SCHC_Ack_on_error::mtuUpgrade - Entering the function");
#endif

#ifdef MYINFO
    Serial.print("SCHC_Ack_on_error::mtuUpgrade - updating the MTU to: ");
    Serial.print(mtu);
    Serial.println(" bytes");
#endif
    _current_L2_MTU = mtu;

#ifdef MYTRACE
    Serial.println("SCHC_Ack_on_error::mtuUpgrade - Leaving the function");
#endif

    return 0;
}

uint8_t SCHC_Ack_on_error::divideInTiles(char *buffer, int len)
{
/* Creates an array of size _nFullTiles x _tileSize to store the message. 
Each row of the array is a tile of tileSize bytes. It also determines 
the number of SCHC windows.*/
#ifdef MYTRACE
    Serial.println("SCHC_Ack_on_error::divideInTiles - Entering the function");
#endif

/* RFC9011
 5.6.2. Uplink Fragmentation: From Device to SCHC Gateway

Last tile: It can be carried in a Regular SCHC Fragment, alone 
in an All-1 SCHC Fragment, or with any of these two methods. 
Implementations must ensure that:
*/
    if((len%_tileSize)==0)
    {
        /* El ultimo tile es del tamaño de _TileSize */
        _nFullTiles = (len/_tileSize)-1;
        _lastTileSize = _tileSize;  
    }
    else
    {
        /* El ultimo tile es menor _TileSize */
        _nFullTiles = (len/_tileSize);
        _lastTileSize = len%_tileSize;
    }

#ifdef MYINFO
    Serial.print("SCHC_Ack_on_error::divideInTiles - Full Tiles number: ");
    Serial.print(_nFullTiles);
    Serial.println(" tiles");
    Serial.print("SCHC_Ack_on_error::divideInTiles - Last Tile Size: ");
    Serial.print(_lastTileSize);
    Serial.println(" bytes");
#endif 

    // memory allocated for elements of rows.
    _tilesArray = new char*[_nFullTiles];           // Liberado en metodo SCHC_Ack_on_error::TX_END_free_resources()

    // memory allocated for  elements of each column.  
    for(int i = 0 ; i < _nFullTiles ; i++ )
    {
        _tilesArray[i] = new char[_tileSize];       // Liberado en metodo SCHC_Ack_on_error::TX_END_free_resources()
    }

    int k=0;
    for(int i=0; i<_nFullTiles; i++)
    {
        for(int j=0; j<_tileSize;j++)
        {
            _tilesArray[i][j] = buffer[k];
            k++;
        }
    }

    _lastTile = new char[_lastTileSize];            // Liberado en metodo SCHC_Ack_on_error::TX_END_free_resources()
    for(int i=0; i<_lastTileSize; i++)
    {
        _lastTile[i] = buffer[k];
        k++;
    }

    /* Numero de ventanas SCHC */
    if(len>(_tileSize*_windowSize*3))
    {
        _nWindows = 4;
    }
    else if(len>(_tileSize*_windowSize*2))
    {
        _nWindows = 3;
    }
    else if (len>(_tileSize*_windowSize))
    {
        _nWindows = 2;
    }
    else
    {
        _nWindows = 1;
    }
#ifdef MYINFO
    Serial.print("SCHC_Ack_on_error::divideInTiles - number of SCHC Windows: ");
    Serial.println(_nWindows);
#endif

    //this->printTileArray();

    /* Calculo del RCS de todo el mensaje. Será usado en el envío de un SCHC All-1 fragment*/
    _rcs = this->calculate_crc32(buffer, len);
#ifdef MYINFO
    Serial.print("SCHC_Ack_on_error::divideInTiles - RCS before to send: ");
    Serial.println(_rcs);
#endif


#ifdef MYTRACE
    Serial.println("SCHC_Ack_on_error::divideInTiles - Leaving the function");
#endif

    return 0;
}

uint8_t SCHC_Ack_on_error::extractTiles(uint8_t firstTileID, uint8_t nTiles, char *buff)
{
#ifdef MYTRACE
    Serial.println("SCHC_Ack_on_error::extractTiles - Entering the function");
#endif

    int k=0;
    for(int i=firstTileID; i<(firstTileID+nTiles); i++)
    {
        for(int j=0;j<_tileSize;j++)
        {
            buff[k] = _tilesArray[i][j];
            k++;
        }
    }

#ifdef MYTRACE
    Serial.println("SCHC_Ack_on_error::extractTiles - Leaving the function");
#endif

    return 0;
}

uint8_t SCHC_Ack_on_error::getCurrentWindow(int tile_ptr)
{
    if(tile_ptr < _windowSize)
    {
        return 0;
    }
    else if(tile_ptr < 2 * _windowSize)
    {
        return 1;
    }
    else if(tile_ptr < 3 * _windowSize)
    {
        return 2;
    }
    else if(tile_ptr < 4 * _windowSize)
    {
        return 3;
    }
    else
    {
        Serial.println("SCHC_Ack_on_error::getCurrentWindow - ERROR: In LoRaWAN, it is not possible that more than (4 * _windowSize)) tiles");
    }
    return 0;
}

uint8_t SCHC_Ack_on_error::getCurrentFCN(int tile_ptr)
{
    if(tile_ptr < _windowSize)
    {
        return (_windowSize - 1 - tile_ptr);
    }
    else if(tile_ptr < 2 * _windowSize)
    {
        return (2*_windowSize - 1 - tile_ptr);
    }
    else if(tile_ptr < 3 * _windowSize)
    {
        return (3*_windowSize - 1 - tile_ptr);
    }
    else if(tile_ptr < 4 * _windowSize)
    {
        return (4*_windowSize - 1 - tile_ptr);
    }
    else
    {
        Serial.println("SCHC_Ack_on_error::getCurrentFCN - ERROR: In LoRaWAN, it is not possible that more than (4 * _windowSize)) tiles");
    }
    return 0;
}

int SCHC_Ack_on_error::getCurrentBitmap_ptr(int tile_ptr)
{
    if(tile_ptr < _windowSize)
    {
        return (tile_ptr);
    }
    else if(tile_ptr < 2 * _windowSize)
    {
        return (tile_ptr - _windowSize);
    }
    else if(tile_ptr < 3 * _windowSize)
    {
        return (tile_ptr - 2*_windowSize);
    }
    else if(tile_ptr < 4 * _windowSize)
    {
        return (tile_ptr - 3*_windowSize);
    }
    else
    {
        Serial.println("SCHC_Ack_on_error::getCurrentBitmap_ptr - ERROR: In LoRaWAN, it is not possible that more than (4 * _windowSize)) tiles");
    }
    return 0;
}

void SCHC_Ack_on_error::printTileArray()
{
    Serial.println("***********************************************");
    for(int i=0; i<_nFullTiles; i++)
    {
        if(i+1<10)
        {
            Serial.print("Tile 0");
            Serial.print(i+1);
            Serial.print(" --> ");
        }
        else
        {
            Serial.print("Tile ");
            Serial.print(i+1);
            Serial.print(" --> ");
        }
        for(int j=0; j<_tileSize;j++)
        {
            Serial.print(_tilesArray[i][j]);
            Serial.print(" ");       
        }
        Serial.println();
    }

    if(_nFullTiles+1<10)
    {
        Serial.print("Tile 0");
        Serial.print(_nFullTiles+1);
        Serial.print(" --> ");
    }
    else
    {
        Serial.print("Tile ");
        Serial.print(_nFullTiles+1);
        Serial.print(" --> ");
    }
    for(int i=0; i<_lastTileSize; i++)
    {
        Serial.print(_lastTile[i]);
        Serial.print(" ");       
    }
    Serial.println();
    Serial.println("***********************************************");
}

uint32_t SCHC_Ack_on_error::calculate_crc32(const char *data, size_t length) 
{
    // Polinomio CRC32 (reflejado)
    const uint32_t polynomial = 0xEDB88320;
    uint32_t crc = 0xFFFFFFFF;

    // Procesar cada byte en el buffer
    for (size_t i = 0; i < length; i++) {
        crc ^= static_cast<uint8_t>(data[i]); // Asegúrate de que el dato sea tratado como uint8_t

        // Procesar los 8 bits del byte
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ polynomial;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc ^ 0xFFFFFFFF;
}
