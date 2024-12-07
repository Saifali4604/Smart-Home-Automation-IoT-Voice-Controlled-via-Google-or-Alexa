// including wifi and blynk libraries
//download Blynk library by Volodymyr
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// namming out the pins

#define ldr 25
#define ir1 33
#define ir2 26
#define relay1 27
#define relay2 12
#define tv 18
#define torch 15
#define lamp 21
#define led 4
#define fanon 5
#define D2 2 // it is used for wifi indication
 
 //wifi setup blynk old version
//enter the authantication token
char auth[] = "rPPG_AdeXiLV3juwbaIEWDfxRsl7NaAF";
//wifi name and password
char ssid[] = "Saif";
char pass[] = "a1234567";

WidgetLCD lcd(V3);
WidgetLED led2(V4);

int tempvalue;
int count=0;
int a=0;
int b=0;
int i=0;
int n=3;
int w=0;
int f=0;
int value = 0;
int time1= 500;
int time2 =2500;
int time3= 1500;
int ledchannel =0;
int freq =5000;
int resolution = 8;

extern "C" //it is used to get the temp data from esp32
{
  uint8_t temprature_sens_read();
}
uint8_t temprature_sens_read();

void setup()
{
      // BlynkEdgent.begin();
       Serial.begin(9600);
       Blynk.begin(auth, ssid, pass);
//blynk old version
    

//wifi indicator
    wifisetup();
//temprature calculator
    temprature();
//pin define
    pinMode(ir1, INPUT);
    pinMode(ir2, INPUT);
    pinMode(ldr, INPUT);
    pinMode(relay1, OUTPUT);
    pinMode(relay2, OUTPUT);
    pinMode(tv, OUTPUT);
    pinMode(torch, OUTPUT);
    pinMode(lamp, OUTPUT);
    pinMode(led, OUTPUT);
    pinMode(fanon,OUTPUT);
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
    lcd.print(0,0,"no. of person:");
    lcd.print(14,0,count);
    lcd.print(0,1,tempvalue);
    lcd.print(3,1,"°C");
    
    ledcSetup(ledchannel, freq, resolution);
    ledcAttachPin(led,ledchannel);
}

BLYNK_WRITE(V1)
{
    int pinvalue = param.asInt();
    if(pinvalue==1)
    {
        digitalWrite(relay1, HIGH);
        a=1;   
    }
    else
    {
        digitalWrite(relay1, LOW);
        a=0;
        
    }
    delay(time2);
}

void loop() {
    
    wifisetup();
    lcd.print(0,0,"no. of person:");
    lcd.print(14,0,count);
    temprature();
    //BlynkEdgent.run();
    Blynk.run();// old version
    irsensor();
    outputa();
    lcd.print(0,1,tempvalue);
    lcd.print(3,1,"°C");
    check();
    notification();
      

} 
 
void wifisetup()
{
    if(WiFi.status() != WL_CONNECTED)
    {
     digitalWrite(D2, LOW);
     //Blynk.begin(auth, ssid, pass);
        w=0;
     }
    else
    {
      digitalWrite(D2, HIGH);
        if(w==0)
      {
         Blynk.notify("Hey,ur {DEVICE_NAME} is now availabie to take care of ur house!");
        w=1;
      }
    }
}
// detects the motion's direction
void irsensor()
{
     while(digitalRead(ir1)==LOW)
    {
        delay(300);
           if(digitalRead(ir2)==LOW)
        {
            count=count+1;
    lcd.print(14,0,count);
            delay(100);
        }
    }
    while(digitalRead(ir2)==LOW)
    {
        delay(300);
        if(digitalRead(ir1)==LOW)
        {
            count=count-1;
            if(count<=-1)
            {
                count=0;
            } 
    lcd.print(14,0,count);
    delay(100);
        }
    }  
}

void temprature()
{
    tempvalue= (temprature_sens_read()-32)/1.8;
    tempvalue =tempvalue-22;
    
}

void check()
{
    if(digitalRead(27)==HIGH)
    {
      led2.on();
    }
    else
    {
      led2.off();
    } 
    if(digitalRead(12)==HIGH)
    {
      Blynk.virtualWrite(V12, HIGH);
    }
    else
    {
     Blynk.virtualWrite(V12, LOW);
    }
    if(digitalRead(21)==HIGH)
    {
      Blynk.virtualWrite(V9, HIGH);
    }
    else
    {
      Blynk.virtualWrite(V9,LOW);
    } 
    if(digitalRead(18)==HIGH)
    {
      Blynk.virtualWrite(V11, HIGH);
    }
    else
    {
      Blynk.virtualWrite(V11, LOW);
    } 
}
// controls the led
void outputa()
{
        if(count==0)
    {
            led2.off();
            ledcWrite(ledchannel,0);
            digitalWrite(relay2, LOW);
         Blynk.virtualWrite(V12, LOW);
        digitalWrite(tv, LOW);
        Blynk.virtualWrite(V11, LOW);
        digitalWrite(lamp, LOW);
        Blynk.virtualWrite(V9, LOW);
            digitalWrite(fanon, LOW);
         Blynk.virtualWrite(V7, LOW);
        if(a==0)
        {
        digitalWrite(relay1, LOW);  
         }  
     }

    else if(digitalRead(ldr)==LOW)
     {
      digitalWrite(relay1, HIGH);  
      }  
   
    }
void notification()
{
    if(count==0&& n<=2)
    {
       Blynk.notify("Hello, sir! Ur room is empty!");
        n=3; 
    }
   else if(count==1 && n==3)
    {
        Blynk.notify("Hello, sir! some one is inside ur room!");
        n=2;
    }
    else if(count>7 && n==2)
    {
        Blynk.notify("Hello, sir! lot of peoples are inside ur room!");
        n=1;
    }
    else if(tempvalue==46)
    {
        Blynk.notify("Hello, sir! Alert!! I feel too hot inside ur room!");
    }
}

   BLYNK_READ(V5) 
{
    Blynk.virtualWrite(V5, tempvalue);
}

 BLYNK_READ(V6)
{
    Blynk.virtualWrite(V6, count);
}
BLYNK_WRITE(V0)
{
    if(f==1)
    {
    int fanspeed = param.asInt();
     if(fanspeed<100)
        {
             Blynk.virtualWrite(V0, 100);
             fanspeed=100;
        }
    ledcWrite(ledchannel,fanspeed);
    delay(100);
        
    }
}

BLYNK_WRITE(V7)
{
    int fanvalue = param.asInt();
    if(fanvalue ==1 && count>0)
    {
        digitalWrite(fanon, HIGH);
        ledcWrite(ledchannel,255);
        delay(time3);
        ledcWrite(ledchannel,100);
        Blynk.virtualWrite(V0, 100);
         f=1;
    }
    else
    { 
        digitalWrite(fanon, LOW);
        ledcWrite(ledchannel,0);
        Blynk.virtualWrite(V0,0);
        f=0;     
    }
}

BLYNK_WRITE(V12)
{
    
    int pin12value = param.asInt();
    if(pin12value==1 && count>0)
    {
        digitalWrite(relay2, HIGH);
        Blynk.virtualWrite(V12, HIGH);
    }
    else
    {
        digitalWrite(relay2, LOW);
        Blynk.virtualWrite(V12, LOW);
    }
        
}

BLYNK_WRITE(V11)
{
   
    int pin18value = param.asInt();
    if(pin18value==1 && count>0)
    {
        digitalWrite(tv, HIGH);
        Blynk.virtualWrite(V11, HIGH);
    }
    else
    {
        digitalWrite(tv, LOW);
        Blynk.virtualWrite(V11, LOW);
    }
        
}

BLYNK_WRITE(V9)
{
    int pin21value = param.asInt();
    if(pin21value==1 &&count>0)
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

BLYNK_WRITE(V10)
{
    int pin10value = param.asInt();
    if(pin10value==1)
    {
        digitalWrite(torch, HIGH);
    }
    else
    {
        digitalWrite(torch, LOW);
    }
        
}
