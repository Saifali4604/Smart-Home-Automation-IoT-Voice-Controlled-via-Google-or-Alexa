#define Firmware_version 3.4.0
#define Build by SAIF_ALI
//This example demonstrates the ESP RainMaker with a standard Switch device.
#include <EEPROM.h>
#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"
#include "DHT.h"
#include <SimpleTimer.h>
//#include <wifi_provisioning/manager.h>
//#include <esp_heap_caps.h>


//for BLE
const char *service_name = "Smart home by saif"; //name of node in BLE
const char *pop = "a1234567"; //password
#define DEFAULT_POWER_MODE true
#define DEFAULT_MODE "HIGH"
#define EEPROM_SIZE 5
#define buzzer 22
#define D2 2 // it is used for wifi indication

// GPIO for Relay (Fan Speed Control)
#define fan_sp4  17
#define fan_sp1  16
#define fan_sp2  4
// GPIO for table device
#define tablefan 25
#define tablelight 26

//#define DEFAULT_RELAY_STATE false
// GPIO for push button
static uint8_t gpio_reset = 0;

// GPIO for Relay (Appliance Control)
static uint8_t DHTPIN = 23;
static uint8_t tubelight = 21;
static uint8_t bulb = 19;
static uint8_t tv = 18;
int fan_time = 100;
int table_byte = 4;
int fan_byte = 3;
int tv_byte = 2;
int bulb_byte = 1;
int tube_byte = 0;
int tf;
int tL;
int i=0;
int wf=0;
int Slider_Value = 0;
int Slider_Value2 = 0;
int curr_speed = 0;

bool tablelight_state;
bool tablestate;
bool fanstate ;
bool tvstate ;
bool bulbstate;
bool tubestate;
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
static Device my_device("Table Light", "custom.device.tablelight");


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
        if (Slider_Value == 4)
        {
          speed_4();
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
  if (strcmp(device_name, "Table Light") == 0)
  {
    
    if (strcmp(param_name, "Power") == 0) {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b? "true" : "false", device_name, param_name);
      tablelight_state = val.val.b;
     if(tablelight_state == false){
        table_light_OFF();
      }
      else{
       table_light_HIGH();
      }
      param->updateAndReport(val);
    }
   else if (strcmp(param_name, "Mode") == 0) 
   {
        const char* mode = val.val.s;
        if (strcmp(mode, "HIGH") == 0)
        {
          table_light_HIGH();
        } 
        else if (strcmp(mode, "MEDIUM") == 0)
        {
          table_light_MEDIUM();
        } 
        else if (strcmp(mode, "LOW") == 0)
        {
          table_light_LOW();
        }
    Serial.printf("\nReceived value = %s for %s - %s\n", val.val.s, device_name, param_name);
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
  pinMode(fan_sp4, OUTPUT);
  pinMode(fan_sp1, OUTPUT);
  pinMode(fan_sp2, OUTPUT);
  pinMode(D2,OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(tablefan,OUTPUT);
  pinMode(tablelight,OUTPUT);
  dht.begin();
  
  Node my_node;
  my_node = RMaker.initNode("EBHA by Saif");
  
  my_node.addDevice(temperature);
  my_node.addDevice(humidity);
  delay(250);
  
  my_tubelight.addCb(write_callback);
  my_node.addDevice(my_tubelight);
  delay(250);

  my_bulb.addCb(write_callback);
  my_node.addDevice(my_bulb);
  delay(250);

  
  my_tv.addCb(write_callback);
  my_node.addDevice(my_tv);
  delay(250);

  //Standard switch device
  my_fan.addCb(write_callback);
  Param speed("Speed", ESP_RMAKER_PARAM_RANGE, value(1), PROP_FLAG_READ | PROP_FLAG_WRITE);
  speed.addUIType(ESP_RMAKER_UI_SLIDER);
  speed.addBounds(value(1), value(4), value(1));
  my_fan.addParam(speed);
  my_node.addDevice(my_fan);
  delay(250);

   //Create custom table light device
  my_device.addNameParam();
  my_device.addPowerParam(DEFAULT_POWER_MODE);
  my_device.assignPrimaryParam(my_device.getParamByName("Power"));
  static const char* modes[] = { "HIGH", "MEDIUM", "LOW" };
  Param mode("Mode", ESP_RMAKER_PARAM_MODE, value("HIGH"), PROP_FLAG_READ | PROP_FLAG_WRITE);
  mode.addValidStrList(modes, 3);
  mode.addUIType(ESP_RMAKER_UI_DROPDOWN);
  my_device.addParam(mode);
  my_device.addCb(write_callback);
  my_node.addDevice(my_device);
  delay(250);
  
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
  tL = EEPROM.read(table_byte);
  tf = EEPROM.read(fan_byte);
  if(tf==4)
   {
    tf=3;
   }
  int fas = EEPROM.read(fan_byte);
  if(fas==0)
    speed_0();
  else if(fas == 1)
    speed_1();
  else if (fas == 2)          
    speed_2();
  else if (fas == 3)
    speed_3();
   else if(fas == 4)
    speed_4();
    
 my_tubelight.updateAndReportParam("Power", tubestate);
 delay(100);
 my_bulb.updateAndReportParam("Power", bulbstate);
 delay(100);
  my_tv.updateAndReportParam("Power", tvstate);
 delay(100); 
 if(tL==0)
 {
   table_light_OFF();
 }
 else if(tL==1)
 {
   table_light_HIGH();
 }
 else if(tL==2)
 {
   table_light_MEDIUM();
 }
 else if(tL==3)
 {
   table_light_LOW();
 }
 
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

    if ((endTime - startTime) > 8000) {
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
        digitalWrite(D2,HIGH);
        delay(100);
        digitalWrite(buzzer,LOW);
        digitalWrite(D2,LOW);
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
   table_fan_OFF();
   digitalWrite(fan_sp4, LOW);
   digitalWrite(fan_sp1, LOW);
   digitalWrite(fan_sp2, LOW);
   
   EEPROM.write(fan_byte, 0);
   EEPROM.commit();
   
  fanstate = false;
  my_fan.updateAndReportParam("Power", fanstate);
}


void speed_1()
{
  //Speed1 Relay On - Fan at speed 1
  Serial.println("SPEED 1");
  curr_speed = 1;
  table_fan_1();
  digitalWrite(fan_sp4, LOW);
  digitalWrite(fan_sp2,LOW);
  digitalWrite(fan_sp1,HIGH);
  
   EEPROM.write(fan_byte, 1);
   EEPROM.commit();
  fanstate = true;
  my_fan.updateAndReportParam("Power", fanstate);
  my_fan.updateAndReportParam("My_Speed", 1);

}

void speed_2()
{
  //Speed2 Relay On - Fan at speed 2
  Serial.println("SPEED 2");
  curr_speed = 2;
  table_fan_2();
  digitalWrite(fan_sp4, LOW);
  digitalWrite(fan_sp1,LOW);
  digitalWrite(fan_sp2,HIGH);
   EEPROM.write(fan_byte, 2);
   EEPROM.commit();
  fanstate = true;
  my_fan.updateAndReportParam("Power", fanstate);
  my_fan.updateAndReportParam("Speed", 2);

}

void speed_3()
{
  //Speed1 & Speed2 Relays On - Fan at speed 3
  Serial.println("SPEED 3");
  curr_speed = 3;
  table_fan_3();
  digitalWrite(fan_sp4,LOW);
  digitalWrite(fan_sp1,HIGH);
  digitalWrite(fan_sp2,HIGH);
   EEPROM.write(fan_byte, 3);
   EEPROM.commit();
  fanstate = true;
  my_fan.updateAndReportParam("Power", fanstate);
  my_fan.updateAndReportParam("Speed", 3);
}

void speed_4()
{
  //Speed1 & Speed2 Relays On - Fan at speed 3
  Serial.println("SPEED 4");
  curr_speed = 4;
  table_fan_3();
  digitalWrite(fan_sp1,LOW);
  digitalWrite(fan_sp2,LOW);
  digitalWrite(fan_sp4, HIGH);
   EEPROM.write(fan_byte, 4);
   EEPROM.commit();
  fanstate = true;
  my_fan.updateAndReportParam("Power", fanstate);
  my_fan.updateAndReportParam("Speed", 4);
}

// Below code is for table fan demonstration
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
// Below code is for table light
void table_light_OFF()
{
    EEPROM.write(table_byte, 0);
    EEPROM.commit();
    tablestate = false;
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

   my_device.updateAndReportParam("Power", tablestate);
}
void table_light_HIGH()
{
   EEPROM.write(table_byte, 1);
   EEPROM.commit();
   tablestate = true;
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

    my_device.updateAndReportParam("Power", tablestate);
    my_device.updateAndReportParam("Mode", "HIGH"); 
}
void table_light_MEDIUM()
{
   EEPROM.write(table_byte, 2);
   EEPROM.commit();
    tablestate = true;
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
     my_device.updateAndReportParam("Power", tablestate);
    my_device.updateAndReportParam("Mode", "MEDIUM"); 
}
void table_light_LOW()
{
   EEPROM.write(table_byte, 3);
   EEPROM.commit();
    tablestate = true;
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
     my_device.updateAndReportParam("Power", tablestate);
    my_device.updateAndReportParam("Mode", "LOW"); 
}
