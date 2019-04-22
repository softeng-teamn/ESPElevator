#include <LiquidCrystal.h>
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"//to disable brownout
#include <ESP32Servo.h>
#include <HTTPClient.h>

const char* ssid     = "Robot";
const char* password = "RobitWPI";
const int ticksPerRev = 64*50;
const int rs = 23, en = 16, d4 = 5, d5 = 18, d6 = 19, d7 = 17;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const int timeBetweenFloorsU = 800;
const int timeBetweenFloorsD = 500;

const int floorOne = 34;
const int floorTwo = 35;
const int floorThree = 32;
const int floorFour = 33;
const int elevatorPin = 26;
const int encoder0PinA = 17;
const int encoder0PinB = 16;
int startTime = 0;
bool doneMoving = true;
int currentLevel = 1; //start on first floor
const int destSize = 15;
int destLevel[destSize];
//time between floors in ms
const int ticksPerFloor = 50;
int vel = 60;//number to add to or subtract from 90 for base speed of elevator
Servo elevator;
int tail = 0;
int head = 0;
int prevTime = 0;
int prevButton = 1;
int prevDB = 1;
void checkCalls();
int n = 0;
int encoder0Pos = 0;
int encoder0PinALast; 
int safetyPin = 27;
String team = "N";
void setup() {
  lcd.begin(16, 2);//display

  Serial.begin(115200);
  
   WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

    WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  //buttons
  pinMode(floorOne, INPUT_PULLUP);
  pinMode(floorTwo, INPUT_PULLUP);
  pinMode(floorThree, INPUT_PULLUP);
  pinMode(floorFour, INPUT_PULLUP);
  pinMode(encoder0PinA, INPUT);
  pinMode(encoder0PinB, INPUT);
  pinMode(safetyPin, OUTPUT);

  encoder0PinALast = digitalRead(encoder0PinA);
  elevator.attach(elevatorPin, 1000, 2000);
  destLevel[head] = 1;
}


void loop()
{
  
  Serial.print("Head = ");
  Serial.print(head);
  Serial.print("\tTail = ");
  Serial.print(tail);
  Serial.print("Current Floor = ");
  Serial.print(currentLevel);

  Serial.print("\tArray: ");
  for(int i = 0; i < destSize; i++){
    Serial.print(destLevel[i]);
    Serial.print(",");  
  }
  Serial.println();
  
 checkCalls(); //see if any buttons are being pressed
 if(millis() > prevTime + 5000){
  digitalWrite(safetyPin, HIGH);
  Serial.println("checking internet DB");
  int f = getNextFloor();
  if(f){
    addToQ(f, true);
    prevDB = f;
  }
  prevTime = millis();
  digitalWrite(safetyPin, LOW);
 }
  
  if(tail != head) { //if a call has been made to another floor
    if(doneMoving){    
      doneMoving = false;
      startTime = millis();
    }
    if(!doneMoving){
     Serial.print("StartTime =");
     Serial.print(startTime);
    int difference = destLevel[(head + 1) % destSize] - currentLevel;//difference between floors
    Serial.print("Diff: ");
    Serial.print(difference);
    int t = timeBetweenFloorsU;
    if(difference < 0){
      t = timeBetweenFloorsD;
    }
    while (millis() < startTime + t * abs(difference)) { //if hasn't traveled dist between floors yet
      if(difference > 0){
        vel = 45;
      }
      else if(difference< 0){
        vel = 15;
      }
      else{
        vel = 0;
      }
      Serial.println("moving Elev");
      elevator.write((difference / abs(difference)) * vel + 90); //diff/abs(diff) gives the sign (up or down)
    }
      if(millis() < startTime + 10000){//wait on floor for 30s
        Serial.println("Waiting on floor");
         elevator.write(90);//don't move if on right floor
      }
      else{//if have waited more than 30s at floor
        doneMoving = true;               
         head = (head + 1) % destSize;        
        currentLevel = destLevel[head];
        Serial.println("ready for next floor");
      }//end else
    }//end if !doneMoving
  }//end if curr != dest
}//end loop

void checkCalls() {
  if (!digitalRead(floorOne)) {
    team = "L";
    /*
      addToQ(1, false);
      prevButton = 1;
      */
  }
  else if (!digitalRead(floorTwo)) {
    team = "M";
    /*
      addToQ(2, false);
      prevButton = 2;
      */
  }
  else if (!digitalRead(floorThree)) {
    team = "O";

    /*
      addToQ(3, false);
      prevButton = 3;
      */
  }
    else if (!digitalRead(floorFour)) {
      team = "P";

  /*
      addToQ(4, false);
      prevButton = 4;
      */
  
  }
}

//defaults to elevator 'S'
int getNextFloor(){
  String req = "elevator=";
  req += team;
  req +=  "L&isESP=true&time=0";
  return post("https://aldenhallpianos.com/softEngGet.php", req);
}

void postCurrentPress(int level){
  String t = "";
  String req = "elevator=";
  req += team;
  req += "L&time=0&isESP=true";
  req += "&floor=";
  req += (String)level;
   
  Serial.print("Posting: ");
  Serial.println(level);
  post("https://aldenhallpianos.com/softEngPost.php", req);
}

int post(String url, String request){
  String response = "";
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    HTTPClient http;

    http.begin(url);  //Specify destination for HTTP request
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");             //Specify content-type header

    int httpResponseCode = http.POST(request);   //Send the actual POST request

    if (httpResponseCode > 0) {

      response = http.getString();                       //Get the response to the request

      //Serial.println(httpResponseCode);   //Print return code
      Serial.print("RESPONSE: ");
      Serial.println(response);           //Print request answer

    } else {

      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);

    }

    http.end();  //Free resources

  } else {

    Serial.println("Error in WiFi connection");

  }
  return response.toInt();
}


void addToQ(int f, bool fromDB){
  if(fromDB){
      if(f != prevDB){
        if(f < -1){
          return;
        }
      tail = (tail + 1) % destSize;  
      Serial.println("addint to Q from DB");
      destLevel[tail] = f;  
  }
  }
  else{
    if(f != prevButton){
      Serial.println("adding to Q from button");
      tail = (tail + 1) % destSize;  
      destLevel[tail] = f;  
  }
  }
}

void updateEncoders(){
    n = digitalRead(encoder0PinA);
    if ((encoder0PinALast == LOW) && (n == HIGH)) {
    if (digitalRead(encoder0PinB) == LOW) {
      encoder0Pos--;
    } else {
      encoder0Pos++;
    }
    Serial.print (encoder0Pos);
    Serial.print ("/");
  }
  encoder0PinALast = n;
}

