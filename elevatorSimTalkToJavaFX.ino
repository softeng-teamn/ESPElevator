#include <LiquidCrystal.h>
#include <WiFi.h>
#include <SimplePacketComs.h>
#include <Esp32SimplePacketComs.h>
#include <wifi/WifiManager.h>
#include <server/NameCheckerServer.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"//to disable brownout
#include "ElevatorServer.h"
#include <ESP32Servo.h>
//#include "TestClass.h"

ElevatorServer e;

const int rs = 23, en = 16, d4 = 5, d5 = 18, d6 = 19, d7 = 17;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int floorOne = 34;
const int floorTwo = 35;
const int floorThree = 32;
const int floorFour = 33;
const int elevatorPin = 26;
int startTime = 0;
extern bool doneMoving;
extern int currentLevel; //start on first floor
extern int destLevel;
//time between floors in ms
const int timeBetweenFloors = 1000;
const int vel = 40;//number to add to or subtract from 90 for base speed of elevator
Servo elevator;

void checkCalls();

//TestClass tc;

UDPSimplePacket coms;
WifiManager manager;

void setup() {
  //tc = new TestClass();
  manager.setup();
  lcd.begin(16, 2);//display
  //buttons
  pinMode(floorOne, INPUT_PULLUP);
  pinMode(floorTwo, INPUT_PULLUP);
  pinMode(floorThree, INPUT_PULLUP);
  pinMode(floorFour, INPUT_PULLUP);
  elevator.attach(elevatorPin, 1000, 2000);
  elevator.write(90);
  //setup talking to javaFX
  String * namePointer = new String("MyRobotName");
  
  coms.attach( new ElevatorServer());
  coms.attach(new NameCheckerServer(namePointer));
  Serial.println("Done setup");
  
}


void loop()
{
 
Serial.print("Dest Level: ");
Serial.print(destLevel);
Serial.print("\tCurrentLevel: ");
Serial.println(currentLevel);

/*
  manager.loop();
  if (manager.getState() == Connected){
    
    coms.server();
  }
  //move elevator to floor
*/

  if (doneMoving) {
    checkCalls();
    elevator.write(90);
  }
  
  else if(currentLevel != destLevel) { //if a call has been made to another floor
    int difference = destLevel - currentLevel;//difference between floors
    if (millis() < startTime + timeBetweenFloors * abs(difference)) { //if hasn't traveled dist between floors yet
      elevator.write((difference / abs(difference)) * vel + 90); //diff/abs(diff) gives the sign (up or down)
    }
    else {//elevator traveled correct distance
      elevator.write(90);//don't move if on right floor
      doneMoving = true;
      currentLevel = destLevel;
    }
  }
  else{//call made to floor elevator is on
    doneMoving = true;
    elevator.write(90);//don't move if on right floor
  }
  delay(15);
  
}

void checkCalls() {
  if (!digitalRead(floorOne)) {
    destLevel = 1;
    doneMoving = false;
    startTime = millis();
  }
  else if (!digitalRead(floorTwo)) {
    destLevel = 2;
    doneMoving = false;
    startTime = millis();
  }
  else if (!digitalRead(floorThree)) {
    destLevel = 3;
    doneMoving = false;
    startTime = millis();
  }
    else if (!digitalRead(floorFour)) {
    destLevel = 4;
    doneMoving = false;
    startTime = millis();
    }
}
