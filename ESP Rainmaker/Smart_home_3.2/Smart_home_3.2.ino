#define Firmware_version 3.4.0
#define Build by SAIF_ALI
//This example demonstrates the ESP RainMaker with a standard Switch device.
#include <EEPROM.h>
#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"
#include "DHT.h"
#include <SimpleTimer.h>
#include <wifi_provisioning/manager.h>
#include <esp_heap_caps.h>


//for BLE
const char *service_name = "Smart_home"; //name of node in BLE
const char *pop = "a1234567"; //password

#define EEPROM_SIZE 5
#define buzzer 22
#define D2 2 // it is used for wifi indication

// GPIO for Relay (Fan Speed Control)
#define fan  17
#define relay1  16
#define relay2  4

//#define DEFAULT_RELAY_STATE false
int i=0;
int wf=0;
// GPIO for push button
static uint8_t gpio_reset = 0;

// GPIO for Relay (Appliance Control)
static uint8_t DHTPIN = 23;
static uint8_t tubelight = 21;
static uint8_t bulb = 19;
static uint8_t tv = 18;

int fan_byte = 3;
int tv_byte = 2;
int bulb_byte = 1;
int tube_byte = 0;

bool fanstate ;
bool tvstate ;
bool bulbstate;
bool tubestate;


int Slider_Value = 0;
int curr_speed = 0;
bool Fan_Switch = 0;
bool wifi_scanning  = 0;
bool tubelight_state ;
bool bulb_state ;
bool tv_state ;

DHT dht(DHTPIN, DHT11);
SimpleTimer Timer;

static TemperatureSensor temperature("Temperature");
static TemperatureSensor humidity("Humidity");
static LightBulb my_tubelight("Tubelight", &tubelight);
static LightBulb my_bulb("Bulb", &bulb);
static Switch my_tv("TV Plug", &tv);
static Fan my_fan("Fan");

void sysProvEvent(arduino_event_t *sys_event)
{
  switch (sys_event->event_id)
  {
    case ARDUINO_EVENT_PROV_START:
      Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on BLE\n", service_name, pop);
      printQR(service_name, pop, "ble");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("\nConnected IP address : ");
      Serial.println(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("\nDisconnected. Connecting to the AP again... ");
      break;
    case ARDUINO_EVENT_PROV_CRED_RECV:
      Serial.println("\nReceived Wi-Fi credentials");
      Serial.print("\tSSID : ");
      Serial.println((const char *) sys_event->event_info.prov_cred_recv.ssid);
      Serial.print("\tPassword : ");
      Serial.println((char const *) sys_event->event_info.prov_cred_recv.password);
      break;
    case ARDUINO_EVENT_PROV_INIT:
      wifi_prov_mgr_disable_auto_stop(10000);
      break;
    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
      Serial.println("Stopping Provisioning!!!");
      wifi_prov_mgr_stop_provisioning();
      break;
  }
}

void write_callback(Device *device, Param *param, const param_val_t val, void *priv_data, write_ctx_t *ctx)
{
  const char *device_name = device->getDeviceName();
  const char *param_name = param->getParamName();

  if (strcmp(device_name, "Fan") == 0)
  {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("\nReceived value = %d for %s - %s\n", val.val.b, device_name, param_name);
      Fan_Switch = val.val.b;
      if (Fan_Switch) {
        speed_1();
      }
      else
        speed_0();
        param->updateAndReport(val);
    }

    else if (strcmp(param_name, "Speed") == 0)
    {
      Serial.printf("\nReceived value = %d for %s - %s\n", val.val.i, device_name, param_name);
      int Slider_Value = val.val.i;
        if (Slider_Value == 1)
        {
          speed_1();
        }
        if (Slider_Value == 2)
        {
          
          speed_2();
        }
        if (Slider_Value == 3)
        {
          speed_3();
        }

     /* if (Slider_Value == 0)
      {
        speed_0();
      }*/
      param->updateAndReport(val);
    }
  }

  if (strcmp(device_name, "Tubelight") == 0)
  {
    Serial.printf("Switch value_1 = %s\n", val.val.b ? "true" : "false");

    if (strcmp(param_name, "Power") == 0) {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      tubelight_state = val.val.b;
      (tubelight_state == false) ? digitalWrite(tubelight, LOW) : digitalWrite(tubelight, HIGH);
      if(tubelight_state == false){
        EEPROM.write(tube_byte, 0);
        EEPROM.commit();
      }
      else{
        EEPROM.write(tube_byte, 1);
        EEPROM.commit();
      }
      param->updateAndReport(val);
    }

  }
  if (strcmp(device_name, "Bulb") == 0) {

    Serial.printf("Switch value_2 = %s\n", val.val.b ? "true" : "false");

    if (strcmp(param_name, "Power") == 0) {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      bulb_state = val.val.b;
      (bulb_state == false) ? digitalWrite(bulb, LOW) : digitalWrite(bulb, HIGH);
      if(bulb_state == false){
        EEPROM.write(bulb_byte, 0);
        EEPROM.commit();
      }
      else{
        EEPROM.write(bulb_byte, 1);
        EEPROM.commit();
      }
      param->updateAndReport(val);
    }
  }
  if (strcmp(device_name, "TV Plug") == 0) {

    Serial.printf("Switch value_3 = %s\n", val.val.b ? "true" : "false");

    if (strcmp(param_name, "Power") == 0) {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
      tv_state = val.val.b;
      (tv_state == false) ? digitalWrite(tv, LOW) : digitalWrite(tv, HIGH);
      if(tv_state == false){
        EEPROM.write(tv_byte, 0);
        EEPROM.commit();
      }
      else{
        EEPROM.write(tv_byte, 1);
        EEPROM.commit();
      }
      param->updateAndReport(val);
    }
  }
}


void setup()
{

  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  // Configure the input GPIOs
  pinMode(gpio_reset, INPUT);
  pinMode(tubelight, OUTPUT);
  pinMode(bulb, OUTPUT);
  pinMode(tv, OUTPUT);
  pinMode(fan, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(D2,OUTPUT);
  pinMode(buzzer, OUTPUT);
  dht.begin();
  
  Node my_node;
  my_node = RMaker.initNode("Smart Home");
  
  my_node.addDevice(temperature);
  my_node.addDevice(humidity);
  delay(500);
  
  my_tubelight.addCb(write_callback);
  my_node.addDevice(my_tubelight);
  delay(500);

  my_bulb.addCb(write_callback);
  my_node.addDevice(my_bulb);
  delay(500);

  
  my_tv.addCb(write_callback);
  my_node.addDevice(my_tv);
  delay(500);

  //Standard switch device
  my_fan.addCb(write_callback);
  Param speed("Speed", ESP_RMAKER_PARAM_RANGE, value(1), PROP_FLAG_READ | PROP_FLAG_WRITE);
  speed.addUIType(ESP_RMAKER_UI_SLIDER);
  speed.addBounds(value(1), value(3), value(1));
  my_fan.addParam(speed);
  my_node.addDevice(my_fan);
  delay(500);
  
  // Timer for Sending Sensor's Data
  Timer.setInterval(3000);
  
  //This is optional
  RMaker.enableOTA(OTA_USING_PARAMS);
  //If you want to enable scheduling, set time zone for your region using setTimeZone().
  //The list of available values are provided here https://rainmaker.espressif.com/docs/time-service.html
  // RMaker.setTimeZone("Asia/Shanghai");
  // Alternatively, enable the Timezone service and let the phone apps set the appropriate timezone
  RMaker.enableTZService();
  RMaker.enableSchedule();

  Serial.printf("\nStarting ESP-RainMaker\n");
  RMaker.start();
  delay(2000);

  WiFi.onEvent(sysProvEvent);
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name);
  for(i=0;i<=2;i++)
     {
      memory();
     }
}

void memory()
{
  
 int tbs = EEPROM.read(tube_byte);
  if(tbs==1){
   tubestate = true;
  }
  else{
   tubestate = false;
  }
  
  int tvs = EEPROM.read(tv_byte);
  if(tvs==1){
   tvstate = true;
  }
  else{
   tvstate = false;
  }
  
  int bus = EEPROM.read(bulb_byte);
  if(bus==1){
    bulbstate = true;
  }
  else{
    bulbstate = false;
  }
      
  digitalWrite(tubelight,tbs); 
  digitalWrite(tv,tvs);
  digitalWrite(bulb,bus);
  int fas = EEPROM.read(fan_byte);
  if(fas==0)
    speed_0();
  else if(fas == 1)
    speed_1();
  else if (fas == 2)          
    speed_2();
  else if (fas == 3)
    speed_3();
    
 my_tubelight.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, tubestate);
 delay(100);
 my_bulb.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, bulbstate);
 delay(100);
  my_tv.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, tvstate);
 delay(100); 
}

void loop()
{

   wifisetup();
   if (Timer.isReady() && wf==1) 
   {                    // Check is ready a second timer
    Send_Sensor();
    Timer.reset();                        // Reset a second timer
   }
   
  // Read GPIO0 (external button to gpio_reset device
  if (digitalRead(gpio_reset) == LOW) {
    //Push button pressed
    Serial.printf("reset Button Pressed!\n");
    // Key debounce handling
    delay(100);
    int startTime = millis();
    while (digitalRead(gpio_reset) == LOW) delay(50);
    int endTime = millis();

    if ((endTime - startTime) > 5000) {
      // If key pressed for more than 5secs, reset all
      Serial.printf("reset to factory.\n");
      digitalWrite(buzzer,HIGH);
      delay(1000);
      digitalWrite(buzzer,LOW);
      RMakerFactoryReset(2);
    }
     else if ((endTime - startTime) > 3000) {
      Serial.printf("Reset Wi-Fi.\n");
      for(i=0;i<=2;i++)
      {
        digitalWrite(buzzer,HIGH);
        delay(100);
        digitalWrite(buzzer,LOW);
        delay(100);
      }
      wifi_scanning = 0;
      // If key pressed for more than 3secs, but less than 10, reset Wi-Fi
      RMakerWiFiReset(2);
    }
  }
  delay(100);
}

void wifisetup()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(D2, LOW);
    delay(500);
    wf=0;
    WiFi.begin();
  }
  else
  {
      digitalWrite(D2, HIGH);
      wf=1;
  }
}

void Send_Sensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  temperature.updateAndReportParam("Temperature", t);
  humidity.updateAndReportParam("Temperature", h);
}
//functions for defineing of speeds

void speed_0()
{
  //All Relays Off - Fan at speed 0
  Serial.println("SPEED 0");
   digitalWrite(fan, LOW);
   digitalWrite(relay1, LOW);
   digitalWrite(relay2, LOW);
   EEPROM.write(fan_byte, 0);
   EEPROM.commit();
  fanstate = false;
  my_fan.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, fanstate);
}

void speed_1()
{
  //Speed1 Relay On - Fan at speed 1
  Serial.println("SPEED 1");
  curr_speed = 1;
   digitalWrite(fan, HIGH);
  digitalWrite(relay1,LOW);
  digitalWrite(relay2,LOW);
   EEPROM.write(fan_byte, 1);
   EEPROM.commit();
  fanstate = true;
  my_fan.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, fanstate);
  my_fan.updateAndReportParam("My_Speed", 1);

}

void speed_2()
{
  //Speed2 Relay On - Fan at speed 2
  Serial.println("SPEED 2");
  curr_speed = 2;
   digitalWrite(fan, HIGH);
  digitalWrite(relay1,HIGH);
  digitalWrite(relay2,LOW);
   EEPROM.write(fan_byte, 2);
   EEPROM.commit();
  fanstate = true;
  my_fan.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, fanstate);
  my_fan.updateAndReportParam("My_Speed", 2);

}

void speed_3()
{
  //Speed1 & Speed2 Relays On - Fan at speed 3
  Serial.println("SPEED 3");
  curr_speed = 3;
  digitalWrite(fan, HIGH);
  digitalWrite(relay1,LOW);
  digitalWrite(relay2,HIGH);
   EEPROM.write(fan_byte, 3);
   EEPROM.commit();
  fanstate = true;
  my_fan.updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, fanstate);
  my_fan.updateAndReportParam("My_Speed", 3);
}
