
#ifndef ELEVATORSERVER_H
#define ELEVATORSERVER_H

#include <WiFi.h>
#include <SimplePacketComs.h>
#include <Esp32SimplePacketComs.h>
#include <wifi/WifiManager.h>

#include <Esp32SimplePacketComs.h>
#include <SimplePacketComs.h>
#include <BNO055SimplePacketComs.h>

class ElevatorServer: public PacketEventAbstract {
  public:
    // Packet ID needs to be set
    ElevatorServer () :
      PacketEventAbstract(1871) // Address of this event
    {
    }
    //User function to be called when a packet comes in
    // Buffer contains data from the packet coming in at the start of the function
    // User data is written into the buffer to send it back
    void event(float * buffer) {
      buffer[0]=37;
      Serial.println("Talking to client");
      
      /*extern int currentLevel;//test if these work.
      extern int destLevel;
      *buffer = (float) currentLevel;
      buffer[1] = (float) destLevel;
      */
    }
};

#endif
