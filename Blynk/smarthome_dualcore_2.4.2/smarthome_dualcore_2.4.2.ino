// including wifi and blynk libraries
//download Blynk library by Volodymyr
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <BlynkSimpleEsp32.h>
#include <Arduino.h>    
#include "SinricPro.h"
#include "SinricProSwitch.h"
#include "SinricProFanUS.h" //for controlling fan 
#include "DHT.h" //temprature DHT11/DHT22
// Uncomment the following line to enable serial debug output
//#define ENABLE_DEBUG
#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif 

// namming out the pins
#define DHTPIN 23 //Tempreture & Humidity
#define DHTTYPE DHT11 //sensor type
#define buzzer 22
#define bulb 21 //touch 13
#define lamp 19 //touch27
#define tv 18   //touch12
#define led 5  // dc fan, uncomment all (dc fan mention) if ur not using dc fan
#define fanon 17 //touch14
#define relay2 16 //AC fan speed 1,2
#define relay3 4  //AC fan speed 3
#define tablefan 25
#define tablelight 26
#define TRIGGER_PIN 0
#define D2 2 // it is used for wifi indication

// SinricPRO authantication for connecting to ur device (provided in ur Sinricpro Account)
#define APP_KEY           "366f968d-a3ad-494e-84d0-41e522fe5dcc"      // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "1de6b64a-a4cc-4a4f-b166-a1daa23d3691-e56c88cf-a240-42e9-8162-9e3be3b70d04"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
#define SWITCH_ID_1       "620baf98923117df0dc64a55"    // Should look like "5dc1564130xxxxxxxxxxxxxx"
#define SWITCH_ID_2       "620bafdd923117df0dc64a95"    // Should look like "5dc1564130xxxxxxxxxxxxxx"
#define FAN_ID            "620f666b923117df0dc7fab3"
//wifi setup blynk old version
// Blynk authantication for connecting to ur device
char auth[] = "rPPG_AdeXiLV3juwbaIEWDfxRsl7NaAF";//enter the authantication token

//wifi name and password
char ssid[] = "";
char pass[] = "";

//char ssid[] = "Nointernet";
//char pass[] = "17701770";

//char ssid[] = "JioDongle_56C155";
//char pass[] = "";

//char ssid[] = "Govt@college";
//char pass[] = "212212212";

//char ssid[] = "Gulmoher24";
//char pass[] = "";

WidgetLCD lcd(V3);
DHT dht(DHTPIN, DHTTYPE);

TaskHandle_t Task1;
int timeout = 90;

int count=0;
int a=0;
int m=0;
int b=0;
int t1=0;
int t2=0;
int t3=0;
int t4=0;
int tf=0;
int tL=0;
int i=0;
int n=3;
int w=0;
int f=0;
int value = 0;
int time1= 500;
int time2 =2500;
int time3= 1500;
int fan_time=100;
int ledchannel =0;
int freq =5000;
int resolution = 8; //2 bit- 2^2=4 i.e 0 to 3 u can give values
int speed_1 = 150;
int speed_2 = 190;
int speed_3 = 255;
int hum;
int temp;

//SinricPro
bool myPowerState = false;
unsigned long lastBtnPress = 0;
// we use a struct to store all states and values for our fan
// fanSpeed (1..3)
struct {
  bool powerState = false;
  int fanSpeed = 1;
} device_state;

/* bool onPowerState(String deviceId, bool &state) 
 *
 * Callback for setPowerState request
 * parameters
 *  String deviceId (r)
 *    contains deviceId (useful if this callback used by multiple devices)
 *  bool &state (r/w)
 *    contains the requested state (true:on / false:off)
 *    must return the new state
 * 
 * return
 *  true if request should be marked as handled correctly / false if not
 */
bool onPowerState1(const String &deviceId, bool &state) {
  digitalWrite(bulb, state);
  if(state ==HIGH)//updating blynk 
    {
      Blynk.virtualWrite(V1, HIGH);
    }
    else
    {
      Blynk.virtualWrite(V1,LOW);
    } 
  return true; // request handled properly
}
bool onPowerState2(const String &deviceId, bool &state) {
  
  digitalWrite(tv, state);
  if(state ==HIGH)//updating blynk 
    {
      Blynk.virtualWrite(V11, HIGH);
    }
    else
    {
      Blynk.virtualWrite(V11,LOW);
    } 
  return true; // request handled properly
}
bool onPowerState(const String &deviceId, bool &state) {
  Serial.printf("Fan turned %s\r\n", state?"on":"off");
  device_state.powerState = state;
  digitalWrite(fanon, state);
  if(state ==HIGH)//updating blynk 
    {
        Blynk.virtualWrite(V7, HIGH);
        digitalWrite(relay2,LOW);
        digitalWrite(relay3,LOW);
        Blynk.virtualWrite(V0,1);
        table_fan_ON();
        /* ledcWrite(ledchannel,speed_3); // dc fan
        delay(time3);
        ledcWrite(ledchannel,speed_1); // dc fan */
    }
    else
    {
        Blynk.virtualWrite(V7,LOW);
     
        digitalWrite(relay2,LOW);
        digitalWrite(relay3,LOW);
        Blynk.virtualWrite(V0,1);
        table_fan_OFF();
        SinricProFanUS &myFan = SinricPro[FAN_ID];
        myFan.sendRangeValueEvent(1);
        // ledcWrite(ledchannel,0); // dc fan
    } 
  return true; // request handled properly
}
// Fan rangeValue is from 1..3
bool onRangeValue(const String &deviceId, int &rangeValue) {
  device_state.fanSpeed = rangeValue;
  sinricpro_fan_controll();
     
  Serial.printf("Fan speed changed to %d\r\n", device_state.fanSpeed);
  return true;
}
// Fan rangeValueDelta is from -3..+3
bool onAdjustRangeValue(const String &deviceId, int &rangeValueDelta) {
  device_state.fanSpeed += rangeValueDelta;
  Serial.printf("Fan speed changed about %i to %d\r\n", rangeValueDelta, device_state.fanSpeed);

  rangeValueDelta = device_state.fanSpeed; // return absolute fan speed
  sinricpro_fan_controll();
  return true;
}

void setup()
{
    Serial.begin(115200);
    /* WiFi.mode(WIFI_STA);
       WiFiManager wm;
          // set configportal timeout
          wm.setConfigPortalTimeout(timeout);
          bool res;
          res = wm.autoConnect("SmartHome_Saif");
          if(!res) 
          {
                // ESP.restart();
          } 
          else 
          { 
              
          }*/
    //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
    xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
                 
   // wifi_manage();
   WiFi.begin();
   WiFi.reconnect();
   Blynk.begin(auth,ssid, pass);//blynk old version
   wifisetup();//wifi indicator
    setupSinricPro();//SinricPro
    dht.begin();
    //pin define
    pinMode(bulb, OUTPUT);
    pinMode(relay2, OUTPUT);
    pinMode(relay3, OUTPUT);
    pinMode(tv, OUTPUT);
    pinMode(lamp, OUTPUT);
    pinMode(led, OUTPUT);
    pinMode(fanon,OUTPUT);
    pinMode(tablefan,OUTPUT);
    pinMode(tablelight,OUTPUT);
    pinMode(TRIGGER_PIN, INPUT_PULLUP);
    pinMode(D2, OUTPUT);
    delay(1000);
    
    lcd.clear();
    lcd.print(2, 0, "SMART HOME");
    lcd.print(4, 1, "PROJECT");
    delay(time2);
    lcd.clear();
    lcd.print(0, 0, "by ,");
    lcd.print(1, 1, "SAIFALI");
    delay(time3);
    lcd.clear();
    lcd.print(4, 0, "WELCOME");
    delay(time3);
    lcd.clear();
    delay(100);
    // temprature display
    temp_humid();
    // Fan speed control
    ledcSetup(ledchannel, freq, resolution); //dc fan
    ledcAttachPin(led,ledchannel);  // dc fan
    digitalWrite(tablefan, LOW);
}

void loop() 
{
    //wifi_manage();
    wifisetup();
    Blynk.run();//blynk old version
    //check_all();
    temp_humid();
    SinricPro.handle();// SinricPro    
    notification();
    auto_fan();
} 

//Task1code: wifi status and auto reconnect
void Task1code( void * pvParameters )
{
 // Serial.print("Task1 running on core ");
 // Serial.println(xPortGetCoreID());
 for(;;)
  {
    if ( digitalRead(TRIGGER_PIN) == LOW)
   {
        WiFi.mode(WIFI_STA); 
        WiFiManager wm;
        //reset settings - for testing
        //wm.resetSettings();
        // set configportal timeout
        wm.setConfigPortalTimeout(timeout);
        bool res;
        res = wm.startConfigPortal("SmartHome_Saif");
        if(!res) 
        {
              // ESP.restart();
        } 
        else 
        { 
            
        }
   }
  } 
}
void wifi_manage()
{
   if ( digitalRead(TRIGGER_PIN) == LOW)
   {
        WiFi.mode(WIFI_STA); 
        WiFiManager wm;
        //reset settings - for testing
        //wm.resetSettings();
        // set configportal timeout
        wm.setConfigPortalTimeout(timeout);
        bool res;
        res = wm.startConfigPortal("SmartHome_Saif");
        if(!res) 
        {
              // ESP.restart();
        } 
        else 
        { 
            
        }
   }

}
 // wifi setup and indication
void wifisetup()
{
    if(WiFi.status() != WL_CONNECTED)
    {
     digitalWrite(D2, LOW);
     WiFi.reconnect();
     Blynk.begin(auth, ssid, pass);
     w=0;
     }
    else
    {
      digitalWrite(D2, HIGH);
       if(w==0)
      {
         Blynk.notify("Hey,ur {DEVICE_NAME} is now availabie to take care of ur house!");
         Blynk.email("Subject","connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
        w=1;
      }
    }
void temp_humid()
{
    hum = dht.readHumidity();
   temp = dht.readTemperature();
   temp = temp -3;
     // temprature display
    
    lcd.print(0,0,"Temp:");
    lcd.print(6,0,temp);
    lcd.print(9,0,"C");  
    lcd.print(0,1,"Humid:");
    lcd.print(7,1,hum);
    lcd.print(10,1,"%"); 
    Blynk.virtualWrite(V4,temp);
    Blynk.virtualWrite(V5,hum);
    delay(100); 
    
}
void auto_fan()
{
 if(temp==40)
    {
      digitalWrite(fanon, HIGH);
        Blynk.virtualWrite(V7, HIGH);
       
        digitalWrite(relay2,LOW);
        digitalWrite(relay3,LOW);
        Blynk.virtualWrite(V0,1);
        table_fan_ON();
        /* ledcWrite(ledchannel,speed_3); // dc fan 
        delay(time3);
        ledcWrite(ledchannel,speed_1); // dc fan */
    }
    else if(temp==46)
    {
      Blynk.virtualWrite(V7, HIGH);
             Blynk.virtualWrite(V0,2);
             digitalWrite(fanon, HIGH);
             digitalWrite(relay2,HIGH);
             digitalWrite(relay3,LOW);
             table_fan_2();
            //ledcWrite(ledchannel,speed_2); // dc fan
    }
    else if(temp==50)
    {
      
         Blynk.virtualWrite(V7, HIGH);
         Blynk.virtualWrite(V0,3);
         digitalWrite(fanon, HIGH);
         digitalWrite(relay2,LOW);
         digitalWrite(relay3,HIGH);
         table_fan_3();
        // ledcWrite(ledchannel,speed_3);  // dc fan
    }
}
// checking the status of all program
void check_all()
{
    //wifisetup();// check wifi status
    //Bulb
    if(digitalRead(21)==HIGH)
    {
      Blynk.virtualWrite(V1, HIGH);
    }
    else
    {
      Blynk.virtualWrite(V1, LOW);
    } 
    //Lamp
    if(digitalRead(19)==HIGH)
    {
      Blynk.virtualWrite(V9, HIGH);
    }
    else
    {
      Blynk.virtualWrite(V9,LOW);
    } 
    // Tv
    if(digitalRead(18)==HIGH)
    {
      Blynk.virtualWrite(V11, HIGH);
    }
    else
    {
      Blynk.virtualWrite(V11, LOW);
    }
     // Fan
    if(digitalRead(17)==HIGH)
    {
      Blynk.virtualWrite(V7, HIGH);
    }
    else
    {
      Blynk.virtualWrite(V7,LOW);
    }        
}



//for sending notifications 
void notification()
{
    if(temp==46)
    {
        Blynk.notify("Hello, sir! Alert!! I feel too hot inside ur room!");
        Blynk.email("Subject","Hello, sir! Alert!! I feel too hot inside ur room!");
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
  SinricPro.begin(APP_KEY, APP_SECRET);
}

void sinricpro_fan_controll()
{
  if(device_state.fanSpeed == 1)
    {
        digitalWrite(fanon, HIGH);
        digitalWrite(relay2,LOW);
        digitalWrite(relay3,LOW);
        Blynk.virtualWrite(V7, HIGH);
        Blynk.virtualWrite(V0,1);
        table_fan_1();
        SinricProFanUS &myFan = SinricPro[FAN_ID];
        myFan.sendPowerStateEvent(true);
        /* ledcWrite(ledchannel,speed_3); //dc fan
        delay(time3);
        ledcWrite(ledchannel,speed_1); // dc fan  */
     }
   else if(device_state.fanSpeed == 2)
     {
        digitalWrite(fanon, HIGH);
        digitalWrite(relay2,HIGH);
        digitalWrite(relay3,LOW);
        Blynk.virtualWrite(V7, HIGH);
        Blynk.virtualWrite(V0,2);
        table_fan_2();
        SinricProFanUS &myFan = SinricPro[FAN_ID];
        myFan.sendPowerStateEvent(true);
        // speed 2
       // ledcWrite(ledchannel,speed_2); // dc fan
     }
     else 
     {
        digitalWrite(fanon, HIGH);
        digitalWrite(relay2,LOW);
        digitalWrite(relay3,HIGH);
        Blynk.virtualWrite(V7, HIGH);
        Blynk.virtualWrite(V0,3);
        table_fan_3();
        SinricProFanUS &myFan = SinricPro[FAN_ID];
        myFan.sendPowerStateEvent(true);
        
        // speed 3
       // ledcWrite(ledchannel,speed_3);  //dc fan
     }
}

void table_fan_ON()
{
    if(tf==0)
    {
      digitalWrite(tablefan, HIGH);
      delay(fan_time);
      digitalWrite(tablefan, LOW);
      delay(fan_time);
      tf=1;
    }    
}
void table_fan_OFF()
{
    if(tf==1)
    { 
      for(i=1;i<=3;i++)
      {
        digitalWrite(tablefan, HIGH);
        delay(fan_time);
        digitalWrite(tablefan, LOW);
        delay(fan_time);
        tf=0;  
      } 
    }
    else if(tf==2)
    {
      for(i=1;i<=2;i++)
      {
        digitalWrite(tablefan, HIGH);
        delay(fan_time);
        digitalWrite(tablefan, LOW);
        delay(fan_time);
        tf=0;  
      } 
    }
    else if(tf==3)
    {
        digitalWrite(tablefan, HIGH);
        delay(fan_time);
        digitalWrite(tablefan, LOW);
        delay(fan_time);
        tf=0;  
    }   
}
void table_fan_1()
{
   if(tf==2)
    { 
      for(i=1;i<=3;i++)
      {
        digitalWrite(tablefan, HIGH);
        delay(fan_time);
        digitalWrite(tablefan, LOW);
        delay(fan_time);
        tf=1;  
      } 
    }
    else if(tf==3)
    { 
      for(i=1;i<=2;i++)
      {
        digitalWrite(tablefan, HIGH);
        delay(fan_time);
        digitalWrite(tablefan, LOW);
        delay(fan_time);
        tf=1;  
      } 
    }
    else if(tf==0)
    { 
        digitalWrite(tablefan, HIGH);
        delay(fan_time);
        digitalWrite(tablefan, LOW);
        delay(fan_time);
        tf=1;  
    }
     
}
void table_fan_2()
{
   if(tf==3)
    { 
      for(i=1;i<=3;i++)
      {
        digitalWrite(tablefan, HIGH);
        delay(fan_time);
        digitalWrite(tablefan, LOW);
        delay(fan_time);
        tf=2;  
      } 
    }
    else if(tf==0)
    { 
      for(i=1;i<=2;i++)
      {
        digitalWrite(tablefan, HIGH);
        delay(fan_time);
        digitalWrite(tablefan, LOW);
        delay(fan_time);
        tf=2;  
      } 
    }
    else if(tf==1)
    { 
        digitalWrite(tablefan, HIGH);
        delay(fan_time);
        digitalWrite(tablefan, LOW);
        delay(fan_time);
        tf=2;  
    }
     
}
void table_fan_3()
{
   if(tf==0)
    { 
      for(i=1;i<=3;i++)
      {
        digitalWrite(tablefan, HIGH);
        delay(fan_time);
        digitalWrite(tablefan, LOW);
        delay(fan_time);
        tf=3;  
      } 
    }
    else if(tf==1)
    { 
      for(i=1;i<=2;i++)
      {
        digitalWrite(tablefan, HIGH);
        delay(fan_time);
        digitalWrite(tablefan, LOW);
        delay(fan_time);
        tf=3;  
      } 
    }
    else if(tf==2)
    { 
        digitalWrite(tablefan, HIGH);
        delay(fan_time);
        digitalWrite(tablefan, LOW);
        delay(fan_time);
        tf=3;  
    }
     
}
void table_light_ON()
{
    if(tL==0)
    {
      digitalWrite(tablelight, HIGH);
      delay(fan_time);
      digitalWrite(tablelight, LOW);
      delay(fan_time);
      tL=1;
    }    
}
void table_light_OFF()
{
    if(tL==1)
    { 
      for(i=1;i<=3;i++)
      {
        digitalWrite(tablelight, HIGH);
        delay(fan_time);
        digitalWrite(tablelight, LOW);
        delay(fan_time);
        tL=0;  
      } 
    }
    else if(tL==2)
    {
      for(i=1;i<=2;i++)
      {
        digitalWrite(tablelight, HIGH);
        delay(fan_time);
        digitalWrite(tablelight, LOW);
        delay(fan_time);
        tL=0;  
      } 
    }
    else if(tL==3)
    {
        digitalWrite(tablelight, HIGH);
        delay(fan_time);
        digitalWrite(tablelight, LOW);
        delay(fan_time);
        tL=0;  
    }   
}
void table_light_HIGH()
{
   if(tL==2)
    { 
      for(i=1;i<=3;i++)
      {
        digitalWrite(tablelight, HIGH);
        delay(fan_time);
        digitalWrite(tablelight, LOW);
        delay(fan_time);
        tL=1;  
      } 
    }
    else if(tL==3)
    { 
      for(i=1;i<=2;i++)
      {
        digitalWrite(tablelight, HIGH);
        delay(fan_time);
        digitalWrite(tablelight, LOW);
        delay(fan_time);
        tL=1;  
      } 
    }
    else if(tL==0)
    { 
        digitalWrite(tablelight, HIGH);
        delay(fan_time);
        digitalWrite(tablelight, LOW);
        delay(fan_time);
        tL=1;  
    }
     
}
void table_light_MEDIUM()
{
   if(tL==3)
    { 
      for(i=1;i<=3;i++)
      {
        digitalWrite(tablelight, HIGH);
        delay(fan_time);
        digitalWrite(tablelight, LOW);
        delay(fan_time);
        tL=2;  
      } 
    }
    else if(tL==0)
    { 
      for(i=1;i<=2;i++)
      {
        digitalWrite(tablelight, HIGH);
        delay(fan_time);
        digitalWrite(tablelight, LOW);
        delay(fan_time);
        tL=2;  
      } 
    }
    else if(tL==1)
    { 
        digitalWrite(tablelight, HIGH);
        delay(fan_time);
        digitalWrite(tablelight, LOW);
        delay(fan_time);
        tL=2;  
    }
     
}
void table_light_LOW()
{
   if(tL==0)
    { 
      for(i=1;i<=3;i++)
      {
        digitalWrite(tablelight, HIGH);
        delay(fan_time);
        digitalWrite(tablelight, LOW);
        delay(fan_time);
        tL=3;  
      } 
    }
    else if(tL==1)
    { 
      for(i=1;i<=2;i++)
      {
        digitalWrite(tablelight, HIGH);
        delay(fan_time);
        digitalWrite(tablelight, LOW);
        delay(fan_time);
        tL=3;  
      } 
    }
    else if(tL==2)
    { 
        digitalWrite(tablelight, HIGH);
        delay(fan_time);
        digitalWrite(tablelight, LOW);
        delay(fan_time);
        tL=3;  
    }
     
}

BLYNK_WRITE(V12)
{
    int pinvalue = param.asInt();
    if(pinvalue==1)
    {
      table_light_ON();
    }
    else
    {
      table_light_OFF();
    }
}
BLYNK_WRITE(V13)
{
    int pinvalue = param.asInt();
    if(pinvalue==1)
    {
      table_light_HIGH();
    }
    else if(pinvalue==2)
    {
      table_light_MEDIUM();
    }
    else
    {
      table_light_LOW();
    }
}
// controlling relay1 by V1
 BLYNK_WRITE(V1)
{
    int pinvalue = param.asInt();
    if(pinvalue==1)
    {
        digitalWrite(bulb, HIGH);
        a=1;   
                  // add devices and callbacks to SinricPro
       SinricProSwitch& mySwitch1 = SinricPro[SWITCH_ID_1];// get Switch1
       mySwitch1.sendPowerStateEvent(true);// set callback function to device  
    }
    else
    {
        digitalWrite(bulb, LOW);
        a=0;
                  // add devices and callbacks to SinricPro
       SinricProSwitch& mySwitch1 = SinricPro[SWITCH_ID_1];// get Switch1
       mySwitch1.sendPowerStateEvent(false);// set callback function to device  
        
    }
}


// controlling fan by V7
BLYNK_WRITE(V7)
{
    int fanvalue = param.asInt();
    if(fanvalue ==1)
    {
        digitalWrite(fanon, HIGH);
        Blynk.virtualWrite(V7, HIGH);
       
        digitalWrite(relay2,LOW);
        digitalWrite(relay3,LOW);
        Blynk.virtualWrite(V0,1);
        table_fan_ON();
        SinricProFanUS &myFan = SinricPro[FAN_ID];
       myFan.sendPowerStateEvent(true);
       myFan.sendRangeValueEvent(1);
        /* ledcWrite(ledchannel,speed_3); // dc fan 
        delay(time3);
        ledcWrite(ledchannel,speed_1); // dc fan */
    }
    else
    { 
        digitalWrite(fanon, LOW);
        Blynk.virtualWrite(V7, LOW);
       
        digitalWrite(relay2,LOW);
        digitalWrite(relay3,LOW);
        Blynk.virtualWrite(V0,1);
        table_fan_OFF();
       SinricProFanUS &myFan = SinricPro[FAN_ID];
       myFan.sendPowerStateEvent(false);
       myFan.sendRangeValueEvent(1);
        // ledcWrite(ledchannel,0); // dc fan
    }
}

// controlling fan speed by V0
BLYNK_WRITE(V0)
{
      int fanspeed = param.asInt();
      if(fanspeed == 1)
      {
        
        digitalWrite(relay2,LOW);
        digitalWrite(relay3,LOW);
        Blynk.virtualWrite(V0,1);
        table_fan_1();
        SinricProFanUS &myFan = SinricPro[FAN_ID];
        myFan.sendRangeValueEvent(1);
       // ledcWrite(ledchannel,speed_1); // dc fan
      }
      else if(fanspeed == 2)
          {
             Blynk.virtualWrite(V7, HIGH);
             Blynk.virtualWrite(V0,2);
             digitalWrite(fanon, HIGH);
             digitalWrite(relay2,HIGH);
             digitalWrite(relay3,LOW);
             table_fan_2();
             SinricProFanUS &myFan = SinricPro[FAN_ID];
             myFan.sendPowerStateEvent(true);
             myFan.sendRangeValueEvent(2);
            //ledcWrite(ledchannel,speed_2); // dc fan
          }
      else
      {
         Blynk.virtualWrite(V7, HIGH);
         Blynk.virtualWrite(V0,3);
         digitalWrite(fanon, HIGH);
         digitalWrite(relay2,LOW);
         digitalWrite(relay3,HIGH);
         table_fan_3();
         SinricProFanUS &myFan = SinricPro[FAN_ID];
         myFan.sendPowerStateEvent(true);
         myFan.sendRangeValueEvent(3);
        // ledcWrite(ledchannel,speed_3);  // dc fan
        
       } 
}

// controlling Tv by V11
BLYNK_WRITE(V11)
{
   
    int pin18value = param.asInt();
    if(pin18value==1)
    {
      digitalWrite(tv, HIGH);
      Blynk.virtualWrite(V11,HIGH);
      SinricProSwitch& mySwitch2 = SinricPro[SWITCH_ID_2];
      mySwitch2.sendPowerStateEvent(true);
    }
    else
    {
      digitalWrite(tv, LOW);
      Blynk.virtualWrite(V11, LOW);
      SinricProSwitch& mySwitch2 = SinricPro[SWITCH_ID_2];
      mySwitch2.sendPowerStateEvent(false);
    }        
}

// controlling lamp by V9
BLYNK_WRITE(V9)
{
    int pin21value = param.asInt();
    if(pin21value==1)
    {
        digitalWrite(lamp, HIGH);
         Blynk.virtualWrite(V9, HIGH);
    }
    else
    {
        digitalWrite(lamp, LOW);
        Blynk.virtualWrite(V9, LOW);
    }
}
