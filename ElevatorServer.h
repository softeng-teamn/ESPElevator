
#ifndef ELEVATORSERVER_H
#define ELEVATORSERVER_H

#include <WiFi.h>
#include <SimplePacketComs.h>
#include <Esp32SimplePacketComs.h>
#include <wifi/WifiManager.h>

#include <Esp32SimplePacketComs.h>
#include <SimplePacketComs.h>
#include <BNO055SimplePacketComs.h>


    int currentLevel = 1;
    int destLevel = 1;
    boolean doneMoving = true;

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
      //buffer [0] = currentLevel, 
      //buffer[1] = dest level,
      //buffer[2] = has the user pressed something on kioskk,
      //buffer[3]  = what the user wants
      buffer[0]=currentLevel;
      buffer[1] = destLevel;
      if(buffer[2]){//if user pressed a button
        buffer[2] = 0;//acknowledge
        destLevel = buffer[2];
        doneMoving = false;//let the elevator know it needs to move now
      }
    }
};

#endif
