#include <Arduino.h>
#include "SCHC_Fragmenter_End_Device.hpp"
#include "Ticker.h" 


/**************** Ticker ************************/
void periodicWakeup();
Ticker myTicker(periodicWakeup, 20000);  // Periodic callback each 20000 milliseconds
int counter = 0;


SCHC_Fragmenter_End_Device* frag;

void setup()
{
#ifndef MYINFO
    Serial.println("=================================");
    Serial.println("setup() - Starting setup function");
#endif 
    // Initialize the built in LED
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // Initialize the connection status LED
    pinMode(LED_CONN, OUTPUT);
    digitalWrite(LED_CONN, LOW);


#ifndef SERIAL_CONNECTION
    // Initialize Serial for debug output
    Serial.begin(115200);

    time_t timeout = millis();
    // On nRF52840 the USB serial is not available immediately
    while (!Serial)
    {
        if ((millis() - timeout) < 5000)
        {
        delay(100);
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        }
        else
        {
        break;
        }
    }
#endif

    digitalWrite(LED_BUILTIN, LOW);

    frag = new SCHC_Fragmenter_End_Device();
    frag->initialize(SCHC_FRAG_PROTOCOL_LORAWAN);

    myTicker.start();
#ifndef MYDEBUG
    Serial.println("=================================");
#endif 
}

void loop()
{
    myTicker.update();
}

void periodicWakeup()
{
#ifndef MYDEBUG
    Serial.println("=================================");
    Serial.println("periodicWakeup() - Starting Periodic function");
#endif 
    myTicker.pause();
    Serial.print("Sending frame now...: ");
    Serial.println(counter);
    digitalWrite(LED_CONN, HIGH);
    delay(50);
    digitalWrite(LED_CONN, LOW);

    counter++;

  
    /* 875 bytes */
    //String s = "The SCHC specification RFC8724 describes generic header compression and fragmentation techniques that can be used on all Low-Power Wide Area Network (LPWAN) technologies defined in RFC8376. Even though those technologies share a great number of common features like star-oriented topologies, network architecture, devices with communications that are mostly quite predictable, etc., they do have some slight differences with respect to payload sizes, reactiveness, etc.SCHC provides a generic framework that enables those devices to communicate on IP networks. However, for efficient performance, some parameters and modes of operation need to be set appropriately for each of the LPWAN technologies.This document describes the parameters and modes of operation when SCHC is used over LoRaWAN networks. The LoRaWAN protocol is specified by the LoRa Alliance in LORAWAN-SPEC.";

    /* 870 bytes */
    String s = "The SCHC specification RFC8724 describes generic header compression and fragmentation techniques that can be used on all Low-Power Wide Area Network (LPWAN) technologies defined in RFC8376. Even though those technologies share a great number of common features like star-oriented topologies, network architecture, devices with communications that are mostly quite predictable, etc., they do have some slight differences with respect to payload sizes, reactiveness, etc.SCHC provides a generic framework that enables those devices to communicate on IP networks. However, for efficient performance, some parameters and modes of operation need to be set appropriately for each of the LPWAN technologies.This document describes the parameters and modes of operation when SCHC is used over LoRaWAN networks. The LoRaWAN protocol is specified by the LoRa Alliance in LORAWAN-";

    Serial.println(s.length()+1);
    char* cstr = new char [s.length()+1];
    strcpy (cstr, s.c_str());

    frag->send(cstr, s.length()+1);

    delete[] cstr;
    myTicker.resume();
#ifndef MYDEBUG
    Serial.println("=================================");
    Serial.println();
#endif 
}
