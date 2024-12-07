#define Firmware_version 2.2.4(sinricpro)
#define Build by SAIF_ALI
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <Arduino.h>    
#include "SinricPro.h"
#include "SinricProSwitch.h"
#include "SinricProFanUS.h" //for controlling fan 

#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif 


#define buzzer 22
#define bulb 21 //touch 13
#define tv 18   //touch12
#define fanon 17 //touch14
#define relay2 16 //AC fan speed 1,2
#define relay3 4  //AC fan speed 3
#define TRIGGER_PIN 0
#define D2 2 // it is used for wifi indication

// SinricPRO authantication for connecting to ur device (provided in ur Sinricpro Account)
#define APP_KEY           "366f968d-a3ad-494e-84d0-41e522fe5dcc"     
#define APP_SECRET        "1de6b64a-a4cc-4a4f-b166-a1daa23d3691-e56c88cf-a240-42e9-8162-9e3be3b70d04"  
#define SWITCH_ID_1       "620baf98923117df0dc64a55"   
#define SWITCH_ID_2       "620bafdd923117df0dc64a95"    
#define FAN_ID            "620f666b923117df0dc7fab3"

//wifi name and password
char ssid[] = "";
char pass[] = "";

AsyncWebServer server(80);


int timeout = 120;
//SinricPro
bool myPowerState = false;
unsigned long lastBtnPress = 0;
// we use a struct to store all states and values for our fan
// fanSpeed (1..3)
struct {
  bool powerState = false;
  int fanSpeed = 1;
} device_state;


bool onPowerState1(const String &deviceId, bool &state) {
  digitalWrite(bulb, state);
  return true; // request handled properly
}
bool onPowerState2(const String &deviceId, bool &state) {
  digitalWrite(tv, state);
  return true; // request handled properly
}
bool onPowerState(const String &deviceId, bool &state) {
  device_state.powerState = state;
  digitalWrite(fanon, state);
  if(state ==HIGH)
    {
        digitalWrite(relay2,LOW);
        digitalWrite(relay3,LOW);
    }
    else
    {
        digitalWrite(relay2,LOW);
        digitalWrite(relay3,LOW);
        SinricProFanUS &myFan = SinricPro[FAN_ID];
        myFan.sendRangeValueEvent(1);
    } 
  return true; // request handled properly
}
// Fan rangeValue is from 1..3
bool onRangeValue(const String &deviceId, int &rangeValue) {
  device_state.fanSpeed = rangeValue;
  sinricpro_fan_controll();
  return true;
}

// Fan rangeValueDelta is from -3..+3
bool onAdjustRangeValue(const String &deviceId, int &rangeValueDelta) {
  device_state.fanSpeed += rangeValueDelta;
  rangeValueDelta = device_state.fanSpeed; // return absolute fan speed
  sinricpro_fan_controll();
  return true;
}

void setup()
{
    Serial.begin(115200);
    //pin define
    pinMode(bulb, OUTPUT);
    pinMode(tv, OUTPUT);
    pinMode(buzzer, OUTPUT);
    pinMode(fanon,OUTPUT);
    pinMode(relay2, OUTPUT);
    pinMode(relay3, OUTPUT);
    pinMode(TRIGGER_PIN, INPUT_PULLUP);
    pinMode(D2, OUTPUT);

   // wifiMulti.addAP("Gulmoher_ent", "Ljkt@2000");
  // wifiMulti.addAP(ssid, pass);
    wifi_manage();
    WiFi.begin();
    
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! This is Smart Home, Inorder to update firmware kindly insert */update* next to ur IP address in URL, Then chose ur file");
     });
     
    AsyncElegantOTA.begin(&server);    // Start AsyncElegantOTA
    server.begin();
    
    wifisetup();//wifi indicator
    setupSinricPro();//SinricPro
    
    delay(1000);
    digitalWrite(buzzer,HIGH);
    delay(200);
    digitalWrite(buzzer,LOW);
}

void loop() 
{
    wifi_manage();
    wifisetup();
    SinricPro.handle();// SinricPro    
} 

void wifi_manage()
{
   if ( digitalRead(TRIGGER_PIN) == LOW)
   {
        digitalWrite(buzzer,HIGH);
        delay(500);
        digitalWrite(buzzer,LOW);
        WiFi.mode(WIFI_STA); 
        WiFiManager wm;
        wm.setConfigPortalTimeout(timeout);
        bool res;
        res = wm.startConfigPortal("SmartHome_Saif");
   }
}
 // wifi setup and indication
void wifisetup()
{
    if(WiFi.status() != WL_CONNECTED)
    {
     //wifiMulti.run();
     digitalWrite(D2, LOW);
     wifi_manage();
     WiFi.reconnect();
     }
    else
    {
      digitalWrite(D2, HIGH);
    }
}

// setup function for SinricPro
void setupSinricPro() {
  // add devices and callbacks to SinricPro
  SinricProSwitch& mySwitch1 = SinricPro[SWITCH_ID_1];// add device to SinricPro
  mySwitch1.onPowerState(onPowerState1);// set callback function to device

  SinricProSwitch& mySwitch2 = SinricPro[SWITCH_ID_2];
  mySwitch2.onPowerState(onPowerState2);

  SinricProFanUS &myFan = SinricPro[FAN_ID];
  myFan.onPowerState(onPowerState);
  myFan.onRangeValue(onRangeValue);
  myFan.onAdjustRangeValue(onAdjustRangeValue);
  // setup SinricPro
  /*SinricPro.onConnected([](){
    digitalWrite(D2, LOW);
    delay(500);
    digitalWrite(D2, HIGH);
    delay(500);
    });*/
 // SinricPro.onDisconnected([](){   });
  // SinricPro.restoreDeviceStates(true); // Uncomment to restore the last known state from the server.
  SinricPro.begin(APP_KEY, APP_SECRET);
}

void sinricpro_fan_controll()
{
  if(device_state.fanSpeed == 1)
    {
        digitalWrite(fanon, HIGH);
        digitalWrite(relay2,LOW);
        digitalWrite(relay3,LOW);
        SinricProFanUS &myFan = SinricPro[FAN_ID];
        myFan.sendPowerStateEvent(true);
     }
   else if(device_state.fanSpeed == 2)
     {
        digitalWrite(fanon, HIGH);
        digitalWrite(relay2,HIGH);
        digitalWrite(relay3,LOW);
        SinricProFanUS &myFan = SinricPro[FAN_ID];
        myFan.sendPowerStateEvent(true);
     }
     else if(device_state.fanSpeed == 3)
     {
        digitalWrite(fanon, HIGH);
        digitalWrite(relay2,LOW);
        digitalWrite(relay3,HIGH);
        SinricProFanUS &myFan = SinricPro[FAN_ID];
        myFan.sendPowerStateEvent(true);
     }
}
