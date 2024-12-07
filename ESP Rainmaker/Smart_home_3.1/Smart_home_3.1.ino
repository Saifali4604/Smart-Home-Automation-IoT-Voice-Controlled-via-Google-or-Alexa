
#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"
#include <WiFiClient.h>
//#include <WiFiManager.h>
#include <SimpleTimer.h>


#define D2 2 // it is used for wifi indication
//#define bulb d3
// BLE Credentils
const char *service_name = "Saif";
const char *pop = "a1234567";

int i=0;
// GPIO
static uint8_t bulb = 3;
static uint8_t gpio_reset = 0;

bool bulb_state ;


bool wifi_connected = 0;


//------------------------------------------- Declaring Devices -----------------------------------------------------//

//The framework provides some standard device types like switch, lightbulb, fan, temperature sensor.

static LightBulb my_bulb("Bulb", &bulb);

void sysProvEvent(arduino_event_t *sys_event)
{
  switch (sys_event->event_id) {
    case ARDUINO_EVENT_PROV_START:
#if CONFIG_IDF_TARGET_ESP32
      Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on BLE\n", service_name, pop);
      printQR(service_name, pop, "ble");
#else
      Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on SoftAP\n", service_name, pop);
      printQR(service_name, pop, "softap");
#endif
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.printf("\nConnected to Wi-Fi!\n");
      wifi_connected = 1;
      digitalWrite(D2, HIGH);
      delay(500);
      break;
    case ARDUINO_EVENT_PROV_CRED_RECV: {
        Serial.println("\nReceived Wi-Fi credentials");
        Serial.print("\tSSID : ");
        Serial.println((const char *) sys_event->event_info.prov_cred_recv.ssid);
        Serial.print("\tPassword : ");
        Serial.println((char const *) sys_event->event_info.prov_cred_recv.password);
        break;
      }
  }
}

void write_callback(Device *device, Param *param, const param_val_t val, void *priv_data, write_ctx_t *ctx)
{
  const char *device_name = device->getDeviceName();
  Serial.println(device_name);
  const char *param_name = param->getParamName();

   
  
 if (strcmp(device_name, "Bulb") == 0)
  {
    if (strcmp(param_name, "Power") == 0)
    {
      Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "false" : "true", device_name, param_name);
      bulb_state = val.val.b;
      if(bulb_state == true)
      {
        digitalWrite(bulb, HIGH);
      }
      else
      {
        digitalWrite(bulb, LOW);
      }
      param->updateAndReport(val);
    }
  }

}

void wifisetup()
{
    if(WiFi.status() != WL_CONNECTED)
    {
     digitalWrite(D2, LOW);
     //setup();
    }
    else
    {
      digitalWrite(D2, HIGH);
    }
}

void setup()
{

  Serial.begin(115200);

  // Configure the input GPIOs
  pinMode(bulb, OUTPUT);
  pinMode(D2,OUTPUT);

  //------------------------------------------- Declaring Node -----------------------------------------------------//
  Node my_node;
  my_node = RMaker.initNode("XAIO Smart home");


  //------------------------------------------- Adding Devices in Node -----------------------------------------------------//
 
  //Standard switch device
  my_bulb.addCb(write_callback);
  my_node.addDevice(my_bulb);

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

  WiFi.onEvent(sysProvEvent);

#if CONFIG_IDF_TARGET_ESP32
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name);
#else
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, pop, service_name);
#endif

}


void loop()
{
//-----------------------------------------------------------  Logic to Reset RainMaker

  // Read GPIO0 (external button to reset device
  if (digitalRead(gpio_reset) == LOW) { //Push button pressed
    Serial.printf("Reset Button Pressed!\n");
    // Key debounce handling
    delay(100);
    int startTime = millis();
    while (digitalRead(gpio_reset) == LOW) delay(50);
    int endTime = millis();

    if ((endTime - startTime) > 10000) {
      // If key pressed for more than 10secs, reset all
      Serial.printf("Reset to factory.\n");
      wifi_connected = 0;
      RMakerFactoryReset(2);
    } else if ((endTime - startTime) > 3000) {
      Serial.printf("Reset Wi-Fi.\n");
      wifi_connected = 0;
      // If key pressed for more than 3secs, but less than 10, reset Wi-Fi
      RMakerWiFiReset(2);
    }
  }
  delay(100);
    wifisetup();
}

