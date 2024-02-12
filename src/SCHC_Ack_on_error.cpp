#include "SCHC_Ack_on_error.hpp"

SCHC_Ack_on_error::SCHC_Ack_on_error()
{
}

uint8_t SCHC_Ack_on_error::init(uint8_t ruleID, uint8_t dTag, uint8_t windowSize, uint8_t tileSize, uint8_t n, uint8_t ackMode, SCHC_Stack_L2 *stack_ptr)
{
#ifndef MYDEBUG
    Serial.println("SCHC_Ack_on_error::init - Entering the function");
#endif
    /* Static SCHC parameters */
    _ruleID = ruleID;
    _dTag = dTag;
    _windowSize = windowSize;
    _nMaxWindows = pow(2,n);
    _nFullTiles = 0;    // in tiles
    _lastTileSize = 0;  // in bytes
    _tileSize = tileSize;      // in bytes
    _ackMode = ackMode;

    /* Static LoRaWAN parameters*/
    _current_L2_MTU = 51;   //SF12
    _stack = stack_ptr;

#ifndef MYDEBUG
    Serial.println("SCHC_Ack_on_error::init - Leaving the function");
#endif

    return 0;
}

uint8_t SCHC_Ack_on_error::execute(char *msg, int len)
{
#ifndef MYDEBUG
    Serial.println("SCHC_Ack_on_error::execute - Entering the function");
#endif
    if(msg!=NULL)
    {
        // definir decoder para recepcion de mensajes
    }
    else
    {
        if(_currentState==STATE_TX_INIT)
        {
            Serial.println("SCHC_Ack_on_error::execute - ERROR: you cannot call the execute() method in STATE_TX_INIT");
        }
        else if(_currentState==STATE_TX_SEND)
        {
            this->TX_SEND_send_fragment();
        }
        else if(_currentState==STATE_TX_WAIT_x_ACK)
        {
            return 1;
        }
    }

#ifndef MYDEBUG
    Serial.println("SCHC_Ack_on_error::execute - Leaving the function");
#endif
    return 0;
}

uint8_t SCHC_Ack_on_error::start(char *buffer, int len)
{
#ifndef MYDEBUG
    Serial.println("SCHC_Ack_on_error::start - Entering the function");
#endif
    /* Dynamic SCHC parameters */
    _currentState = STATE_TX_INIT;
    _currentWindow = 0;
    _currentFcn = (_windowSize)-1;
    _currentBitmap_ptr = 0;
    _currentTile_ptr = 0;

    uint8_t res;
    res = this->divideInTiles(buffer, len);
    if(res==1)
    {
        Serial.println("SCHC_Ack_on_error::start - ERROR when splitting the message into tiles");
    }

    res = this->createBitmapArray();
    if(res==1)
    {
        Serial.println("SCHC_Ack_on_error::start - ERROR creating Bitmap Array");
    }

    /*
    RFC8724
    https://www.rfc-editor.org/rfc/rfc8724.html#name-ack-on-error-mode

     8.4.3.1. Sender Behavior

    A Regular SCHC Fragment message carries in its payload one or more tiles. 
    If more than one tile is carried in one Regular SCHC Fragment:
    the selected tiles MUST be contiguous in the original SCHC Packet, and
    they MUST be placed in the SCHC Fragment Payload adjacent to one another, 
    in the order they appear in the SCHC Packet, from the start of the SCHC Packet 
    toward its end.
    */
    _currentState = STATE_TX_SEND;
    res = this->execute();
#ifndef MYDEBUG
    Serial.println("SCHC_Ack_on_error::start - Leaving the function");
#endif
    return res;
}

uint8_t SCHC_Ack_on_error::mtuUpgrade(int mtu)
{
#ifndef MYDEBUG
    Serial.println("SCHC_Ack_on_error::mtuUpgrade - Entering the function");
#endif

#ifndef MYINFO
    Serial.print("SCHC_Ack_on_error::mtuUpgrade - updating the MTU to: ");
    Serial.print(mtu);
    Serial.println(" bytes");
#endif
    _current_L2_MTU = mtu;

#ifndef MYDEBUG
    Serial.println("SCHC_Ack_on_error::mtuUpgrade - Leaving the function");
#endif

    return 0;
}

uint8_t SCHC_Ack_on_error::TX_SEND_send_fragment()
{
#ifndef MYDEBUG
    Serial.println("SCHC_Ack_on_error::TX_SEND_send_fragment - Entering the function");
#endif

    //printTileArray();

    /* Numero de tiles que se pueden enviar un un payload */
    int payload_available_in_bytes = _current_L2_MTU - 1; // MTU = SCHC header + SCHC payload
    int payload_available_in_tiles = payload_available_in_bytes/_tileSize;

    /* Variables temporales */
    int n_tiles_to_send = 0;        // numero de tiles a enviar
    int newFcn = 0;                 // almacena el nuevo valor de FCN que será usado en el proximo envío
    int newWindow = 0;              // almacena el nuevo valor de W que será usado en el proximo envío
    uint8_t newBitmap_ptr = 0;      // almacena el nuevo valor de _currentBitmap_ptr que será usado en el proximo envío
    int n_remaining_win_tiles = 0;  // n de tiles restantes por enviar (usado en el modo de confirmacion por ventana) 
    int n_all_remaining_tiles = 0;  // n de tiles restantes por enviar (usado en el modo de confirmacion por sesion)
 
    if(_ackMode==ACK_MODE_ACK_END_WIN)
    {
        n_remaining_win_tiles = _currentFcn + 1;                // numero de tiles de la ventana que faltan por enviar
        if(n_remaining_win_tiles>payload_available_in_tiles)
        {
            /* Si la cantidad de tiles que faltan por enviar 
            para la ventana i-esima no alcanza en la MTU 
            significa que aun NO finaliza la ventana i-esima
            */
            n_tiles_to_send = payload_available_in_tiles;

            newBitmap_ptr = this->setBitmapArray(_currentTile_ptr, n_tiles_to_send);
            newFcn = _currentFcn - n_tiles_to_send;
            newWindow = _currentWindow;
            _currentState = STATE_TX_SEND;
        }
        else
        {
            /* Si la cantidad de tiles que faltan por enviar 
            para la ventana i-esima SI alcanza en la MTU 
            significa que este es el ultimo envio para la 
            ventana i-esima
            */
            n_tiles_to_send = n_remaining_win_tiles;

            newBitmap_ptr = this->setBitmapArray(_currentTile_ptr, n_tiles_to_send);
            newFcn = (_windowSize)-1;
            newWindow = _currentWindow + 1;
            _currentState = STATE_TX_WAIT_x_ACK;
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

            newBitmap_ptr = this->setBitmapArray(_currentTile_ptr, n_tiles_to_send);
            newFcn = this->getCurrentFCN(_currentTile_ptr + n_tiles_to_send);
            newWindow = this->getCurrentWindow(_currentTile_ptr + n_tiles_to_send);
            _currentState = STATE_TX_SEND;
        }
        else
        {
            /* Si la cantidad de tiles que faltan por enviar 
            para la sesion SI alcanza en la MTU 
            significa que estos son los ultimos Full Tiles
            */
            n_tiles_to_send = n_all_remaining_tiles;

            newBitmap_ptr = this->setBitmapArray(_currentTile_ptr, n_tiles_to_send);
            newFcn = _windowSize;
            newWindow = this->getCurrentWindow(_currentTile_ptr + n_tiles_to_send);
            _currentState = STATE_TX_WAIT_x_ACK;
        }        
    }

    /* Numero de tiles que se pueden enviar */
//    if(_currentWindow<(_nWindows-1))   /* tiles a enviar en una ventana que NO es la ultima*/



    /* buffer que almacena todos los tiles que se van a enviar */
    int payload_len = n_tiles_to_send * _tileSize;  // in bytes
    char payload[payload_len];

    extractTiles(_currentTile_ptr, n_tiles_to_send, payload);

    /* Crea un mensaje SCHC en formato hexadecimal */
    SCHC_Message msg;
    char msgBuffer[(n_tiles_to_send * _tileSize) + 1];
    int msgBuffer_len = msg.createRegularFragment(_ruleID, _dTag, _currentWindow, _currentFcn, payload, payload_len, msgBuffer);
   
    msg.printMsg(SCHC_FRAG_PROTOCOL_LORAWAN, SCHC_REGULAR_FRAGMENT_MSG, msgBuffer, msgBuffer_len);    
    printCurrentBitmap(_currentWindow);
    uint8_t res = _stack->send_frame(_ruleID, msgBuffer, msgBuffer_len);
    if(res==1)
    {
        Serial.println("SCHC_Ack_on_error::sendRegularFragment - ERROR sending L2 frame");
        return 1;
    }

    _currentBitmap_ptr = newBitmap_ptr;
    _currentFcn = newFcn;
    _currentWindow = newWindow;
    _currentTile_ptr = _currentTile_ptr + n_tiles_to_send;
    this->execute();
    
#ifndef MYDEBUG
    Serial.println("SCHC_Ack_on_error::TX_SEND_send_fragment - Leaving the function");
#endif

    return 0;
}

uint8_t SCHC_Ack_on_error::divideInTiles(char *buffer, int len)
{
#ifndef MYDEBUG
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

#ifndef MYINFO
    Serial.print("SCHC_Ack_on_error::divideInTiles - Full Tiles number: ");
    Serial.print(_nFullTiles);
    Serial.println(" tiles");
    Serial.print("SCHC_Ack_on_error::divideInTiles - Last Tile Size: ");
    Serial.print(_lastTileSize);
    Serial.println(" bytes");
#endif 

    // memory allocated for elements of rows.
    _tilesArray = new char*[_nFullTiles];

    // memory allocated for  elements of each column.  
    for(int i = 0 ; i < _nFullTiles ; i++ )
    {
        _tilesArray[i] = new char[_tileSize];
    }

    uint8_t k=0;
    for(int i=0; i<_nFullTiles; i++)
    {
        for(int j=0; j<_tileSize;j++)
        {
            _tilesArray[i][j] = buffer[k];
            k++;
        }
    }

    _lastTile = new char[_lastTileSize];
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
#ifndef MYINFO
    Serial.print("SCHC_Ack_on_error::divideInTiles - SCHC Windows number: ");
    Serial.println(_nWindows);
#endif


    // // free the allocated memory 
    // for( uint8_t i = 0 ; i < len ; i++ ) {
    //     delete [] _tilesArray[i];
    // }
    // delete [] _tilesArray;

#ifndef MYDEBUG
    Serial.println("SCHC_Ack_on_error::divideInTiles - Leaving the function");
#endif

    return 0;
}

uint8_t SCHC_Ack_on_error::extractTiles(uint8_t firstTileID, uint8_t nTiles, char *buff)
{
#ifndef MYDEBUG
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

#ifndef MYDEBUG
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

uint8_t SCHC_Ack_on_error::setBitmapArray(int tile_ptr, int tile_sent)
{
    /* Se obtiene la posicion al Bitmap Array
    a partir de la posicion del tile que se está enviando */
    uint8_t bitmap_ptr = this->getCurrentBitmap_ptr(tile_ptr);

    if((bitmap_ptr + tile_sent) <= _windowSize)
    {
        /* En este caso todos los tiles enviados
        pertenecen a la misma ventana */
        for(int i=bitmap_ptr; i<(bitmap_ptr + tile_sent); i++)
        {
            _currentBitmapArray[_currentWindow][i] = true;
        }
        return bitmap_ptr + tile_sent;
    }
    else
    {
        /* En este caso los tiles enviados
        pertenecen a la misma ventana (i) y a la (i+1)*/
        for(int i=bitmap_ptr; i<(_windowSize); i++)
        {
            Serial.println(i);
            _currentBitmapArray[_currentWindow][i] = true;
        }

        for(int i=0; i<((bitmap_ptr+tile_sent)-(_windowSize)); i++)
        {
            Serial.println(i);
            _currentBitmapArray[_currentWindow+1][i] = true;
        }
        return (bitmap_ptr+tile_sent)-(_windowSize);
    }


    return 0;
}

uint8_t SCHC_Ack_on_error::getCurrentBitmap_ptr(int tile_ptr)
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

uint8_t SCHC_Ack_on_error::createBitmapArray()
{
    // memory allocated for elements of rows.
    _currentBitmapArray = new bool*[_nWindows];

    // memory allocated for  elements of each column.  
    for(int i = 0 ; i < _nWindows ; i++ )
    {
        _currentBitmapArray[i] = new bool[_windowSize];
    }

    int k=0;
    for(int i=0; i<_nWindows; i++)
    {
        for(int j=0; j<_windowSize;j++)
        {
            _currentBitmapArray[i][j] = false;
            k++;
        }
    }

    return 0;
}

void SCHC_Ack_on_error::printCurrentBitmap(uint8_t nWindow)
{
    Serial.print(" - Bitmap: "); 
    for(int i=0; i<_windowSize; i++)
    {
        if(_currentBitmapArray[nWindow][i])
        {
            Serial.print("1");
        }
        else
        {
            Serial.print("0");
        }
    }
    Serial.println();
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

void SCHC_Ack_on_error::printBitmapArray()
{
    for(int i=0; i<_nWindows; i++)
    {
        for(int j=0; j<_windowSize; j++)
        {
            if(_currentBitmapArray[i][j])
            {
                Serial.print("1");
            }
            else
            {
                Serial.print("0");
            }
        }
        Serial.println();        
    }
}
