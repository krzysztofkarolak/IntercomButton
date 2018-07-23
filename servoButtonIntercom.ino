/*
 * Pushing Intercom Button Using Servo
 * 
 * PHYSICAL PINOUT
 * 
 * 
 * VIRTUAL PINOUT
 * 
 * V20 - Servo Status Log
 * V21 - Enable/Disable Push Notifications
 * V30 - Servo Manual Open/Close
 * V31 - Servo Auto Open
 * 
 */


#include <Servo.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include "arduino_secrets.h" // Load passwords and API Keys

//#define BLYNK_PRINT Serial
#define DEVICE_NAME "EXAMPLE_DEVICE"

char auth[] = SECRET_APIKEY; // Blynk Token

/* Blynk Widgets Init */
WidgetRTC rtc;

/* External Devices Objects Init */
Servo myservo;

/* Pin Assignment */
int servoPin = 10;

/* Custom variables */
int servoCloseAngle = 0, servoOpenAngle = 80;
int servoOpenAutoTime = 2000;
bool isServoOpen = false;
bool notifyByPush = false;


void LogAction(int actionType) {
  //0 - open servo, 1 - close servo, 2 - auto servo
  switch(actionType) {
    case 0:
      Blynk.virtualWrite(V20, 1);
      if(notifyByPush) {
        Blynk.notify("Intercom opened the door.");
      }
    break;
    case 1:
      Blynk.virtualWrite(V20, 0);
      break;
  }
}


void OpenClosePos(bool state) {
  if(state) {
    if(!isServoOpen) {
      //press the button
      myservo.attach(servoPin); 
      delay(20);
      myservo.write(servoOpenAngle);
      isServoOpen = true;
      LogAction(0);
    }
  }
  else {
      //release the button
      myservo.write(servoCloseAngle);
      delay(500);
      myservo.detach();
      isServoOpen = false;
      LogAction(1);
  }
}

void OpenCloseAuto() {
  if(!isServoOpen) {
    OpenClosePos(true);
    delay(servoOpenAutoTime);
    OpenClosePos(false);
  }
}

BLYNK_WRITE(V21) {
  if(param.asInt()) {
    notifyByPush = true;
  }
  else {
    notifyByPush = false;
  }
}

BLYNK_WRITE(V30) {
  if(param.asInt()==SECRET_MANUALOPEN) {
    OpenClosePos(true);
  }
  else if(param.asInt()==0) {
    OpenClosePos(false);
  }
  else {
    Blynk.notify("Unauthorized Intercom access! Wrong Manual Open value.");
  }
}

BLYNK_WRITE(V31) {
  if(param.asInt()==SECRET_AUTOOPEN) {
    OpenCloseAuto();
  }
  else if(param.asInt()!=0) {
    Blynk.notify("Unauthorized Intercom access! Wrong Auto Open value.");
  }
}


void setup() {

  /* Servo Init */
  myservo.attach(servoPin);
  //go to default position
  myservo.write(0);
  delay(600);
  myservo.detach();

  /* Blynk Init */
  Blynk.begin(auth, SECRET_SSID, SECRET_PASS, SECRET_SERVER_URL);
  int myTimeout = millis() / 1000;
  while (Blynk.connect() == false) { // try to connect to server for 10 seconds
    if((millis() / 1000) > myTimeout + 8){ // try local if not connected within 9 seconds
       break;
    }
  }
  rtc.begin();

   /* ----------- OTA ------------- */
  ArduinoOTA.setHostname(DEVICE_NAME);
  ArduinoOTA.onStart([]() {
   // Blynk.disconnect();
  });
  ArduinoOTA.begin();
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V21);
}

void loop() {
  Blynk.run();
  yield();
  ArduinoOTA.handle();
}

