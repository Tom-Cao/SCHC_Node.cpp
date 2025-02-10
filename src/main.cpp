#include <Arduino.h>
#include "SCHC_Node_Fragmenter.hpp"
#include "Ticker.h" 

/*** Ticker ***/
void periodicWakeup();
Ticker myTicker(periodicWakeup, 30000);  // Periodic callback each 20000 milliseconds
int counter = 0;

SCHC_Node_Fragmenter frag;

void setup()
{
#ifdef MYINFO
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
            //delay(100);
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        }
        else
        {
        break;
        }
    }
#endif

    digitalWrite(LED_BUILTIN, LOW);

    
    frag.initialize(SCHC_FRAG_LORAWAN);

    myTicker.start();

#ifdef MYDEBUG
    Serial.println("=================================");
#endif 
}

void loop()
{
    myTicker.update();
}

void periodicWakeup()
{
#ifdef MYINFO
    Serial.println("=================================");
    Serial.println("periodicWakeup() - Starting Periodic function");
#endif 

    myTicker.pause();
    Serial.print("Sending frame now...: ");
    Serial.println(counter);
    digitalWrite(LED_CONN, HIGH);
    digitalWrite(LED_CONN, LOW);
    digitalWrite(LED_CONN, HIGH);
    digitalWrite(LED_CONN, LOW);
    digitalWrite(LED_CONN, HIGH);
    digitalWrite(LED_CONN, LOW);
    digitalWrite(LED_CONN, HIGH);
    digitalWrite(LED_CONN, LOW);
    digitalWrite(LED_CONN, HIGH);
    digitalWrite(LED_CONN, LOW);
    digitalWrite(LED_CONN, HIGH);
    digitalWrite(LED_CONN, LOW);

    counter++;

  
    /* 875 bytes */
    //String s = "The SCHC specification RFC8724 describes generic header compression and fragmentation techniques that can be used on all Low-Power Wide Area Network (LPWAN) technologies defined in RFC8376. Even though those technologies share a great number of common features like star-oriented topologies, network architecture, devices with communications that are mostly quite predictable, etc., they do have some slight differences with respect to payload sizes, reactiveness, etc.SCHC provides a generic framework that enables those devices to communicate on IP networks. However, for efficient performance, some parameters and modes of operation need to be set appropriately for each of the LPWAN technologies.This document describes the parameters and modes of operation when SCHC is used over LoRaWAN networks. The LoRaWAN protocol is specified by the LoRa Alliance in LORAWAN-SPEC.";

    /* 870 bytes */
    String s = "The SCHC specification RFC8724 describes generic header compression and fragmentation techniques that can be used on all Low-Power Wide Area Network (LPWAN) technologies defined in RFC8376. Even though those technologies share a great number of common features like star-oriented topologies, network architecture, devices with communications that are mostly quite predictable, etc., they do have some slight differences with respect to payload sizes, reactiveness, etc.SCHC provides a generic framework that enables those devices to communicate on IP networks. However, for efficient performance, some parameters and modes of operation need to be set appropriately for each of the LPWAN technologies.This document describes the parameters and modes of operation when SCHC is used over LoRaWAN networks. The LoRaWAN protocol is specified by the LoRa Alliance in LORAWAN-.";

    /* 2004 bytes */
    //String s = "The Static Context Header Compression and fragmentation (SCHC) specification (RFC 8724) describes generic header compression and fragmentation techniques for Low-Power Wide Area Network (LPWAN) technologies. SCHC is a generic mechanism designed for great flexibility so that it can be adapted for any of the LPWAN technologies. This document defines a profile of SCHC (RFC 8724) for use in LoRaWAN networks and provides elements such as efficient parameterization and modes of operation. This is an Internet Standards Track document. This document is a product of the Internet Engineering Task Force (IETF). It represents the consensus of the IETF community. It has received public review and has been approved for publication by the Internet Engineering Steering Group (IESG). Further information on Internet Standards is available in Section 2 of RFC 7841. Information about the current status of this document, any errata, and how to provide feedback on it may be obtained at https://www.rfc-editor.org/info/rfc9011. Copyright (c) 2021 IETF Trust and the persons identified as the document authors. All rights reserved. The SCHC specification [RFC8724] describes generic header compression and fragmentation techniques that can be used on all Low-Power Wide Area Network (LPWAN) technologies defined in [RFC8376]. Even though those technologies share a great number of common features like star-oriented topologies, network architecture, devices with communications that are mostly quite predictable, etc., they do have some slight differences with respect to payload sizes, reactiveness, etc.SCHC provides a generic framework that enables those devices to communicate on IP networks. However, for efficient performance, some parameters and modes of operation need to be set appropriately for each of the LPWAN technologies. This document describes the parameters and modes of operation when SCHC is used over LoRaWAN networks. The LoRaWAN protocol is specified by the LoRa Alliance in [LORAWAN-SPEC].";

    Serial.println(s.length());
    char cstr[s.length()];
    strcpy(cstr, s.c_str());

    frag.send(cstr, s.length());

    myTicker.resume();
#ifdef MYINFO
    Serial.println("=================================");
    Serial.println();
#endif 
}
