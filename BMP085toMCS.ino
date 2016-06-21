/*==========Project Name：Arduino Uno+W5100+BMP085 with MCS =============*/
//簡述：使用Arduino Uno版和乙太網路模塊W5100傳送BMP085的環境感測模組的資料到MCS雲端服務器。
//MCS:Media Cloud Sandbox，為一個物聯網雲端平台服務端。(https://mcs.mediatek.com/zh-TW/)
/*============================參考資料===================================*/
//Library參考資料BMP085為Adafruit_BMP085.h
//BMP085函式庫：https://github.com/adafruit/Adafruit-BMP085-Library
//程式碼參考資料文章：
//1.將溫溼度感測器的數值送到MCS | 自製烘衣機控制器[建構中...] - GitBook
//  https://kuochunchang.gitbooks.io/-make-a-dryer-controller/content/iteration1/1.4.html
//2.Cooper Maa: 同時執行多個活動與TimedAction 函式庫簡介：
//  http://coopermaa2nd.blogspot.tw/2011/04/timedaction.html
//3.[LinkIt One 教學] 連接LinkIt ONE 到MediaTek Cloud Sandbox
//  http://blog.cavedu.com/技術交流/linkit-one-教學-連接-linkit-one-到-mediatek-cloud-sandbox/
/*============================Project Author=============================*/
//Building Author: Jeff Ku 
//Building Date: Jun/16 2016
//
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <Ethernet.h>
#define DEVICE_KEY "iCGTJyygtA43kSlz"   //MCS Device Key
#define DEVICE_ID "D1WPYX1T"   //MCS Device ID 
#define SITE_URL "api.mediatek.com"   //API

/*************************************************** 
  This is an example for the BMP085 Barometric Pressure & Temp Sensor

  Designed specifically to work with the Adafruit BMP085 Breakout 
  ----> https://www.adafruit.com/products/391

  These displays use I2C to communicate, 2 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

// Connect VCC of the BMP085 sensor to 3.3V (NOT 5.0V!)
// Connect GND to Ground
// Connect SCL to i2c clock - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 5
// Connect SDA to i2c data - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 4
// EOC is not used, it signifies an end of conversion
// XCLR is a reset pin, also not used here

//Define mac, ip, dnServer, gateway, subnet
  //byte mac[]= { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  IPAddress ip(192,168,0,105);
  byte mac[]={0x00, 0x1C, 0x42, 0x4B, 0x08, 0xFB};
  IPAddress dnServer(8,8,8,8);
  IPAddress gateway(192,168,0,99);
  IPAddress subnet(255,255,255,0);
  
//Declaration for EthernetClient class Variable 
  EthernetClient client;
 
short Btemperature;   //BMP085's temperature variable 
long Bpressure;   //BMP085's pressure variable

Adafruit_BMP085 bmp;

long previousTimeTemperature = 0;  //用來保存前一次的壓力更新狀態的時間
long previousTimePressure = 0;    //用來保存前一次溫度更新狀態的的時間
long previousTimeLocal = 0;      //用來保存前一次的本機環境狀態的時間
long interval = 1000;           //時間間隔

  
void setup() {
  Serial.begin(9600);
  Serial.println("\n[UNO+W5100+BMP085]");
  Ethernet.begin(mac,ip,dnServer,gateway,subnet);
  if(Ethernet.begin(mac)==0)
    {
    Serial.println("Failed to configure Ethernet using DHCP");
    for(;;)
    ;  
    }
    else
    {
    Serial.println("Ethernet configureation OK");  
    } 
  if (!bmp.begin()) {
	Serial.println("Could not find a valid BMP085 sensor, check wiring!");
	while (1) {}
  }
}


//Define a boolean Variable for disconnection messenge 
  boolean disconnectedMsg = false;
  
void loop() 
{
    checkTemperature();
    checkPressure();
    checkLocalEnviorment();
}

void checkLocalEnviorment()
{
    if (millis()-previousTimeLocal>interval)
    {
     printLocalEnviorment();
     previousTimeLocal=millis(); 
    }  
}

void printLocalEnviorment()
{    
    Serial.print("Temperature = ");
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");
    
    Serial.print("Pressure = ");
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");
    
    // Calculate altitude assuming 'standard' barometric
    // pressure of 1013.25 millibar = 101325 Pascal
    Serial.print("Altitude = ");
    Serial.print(bmp.readAltitude());
    Serial.println(" meters");

    Serial.print("Pressure at sealevel (calculated) = ");
    Serial.print(bmp.readSealevelPressure());
    Serial.println(" Pa");

    // you can get a more precise measurement of altitude
    // if you know the current sea level pressure which will
    // vary with weather and such. If it is 1015 millibars
    // that is equal to 101500 Pascals.
    Serial.print("Real altitude = ");
    Serial.print(bmp.readAltitude(101500));
    Serial.println(" meters");
    
    Serial.println();
    delay(5000);
}

void checkTemperature()
{
    //檢查是否已經超過間隔時間
    //是的話，就執行動作並且記錄更新時間
    //millis()是擷取現在時間的函數
    if (millis()-previousTimeTemperature>interval)
    {
     clientPrintTemperature();
     previousTimeTemperature=millis(); 
    }
}
void clientPrintTemperature()
{
    Btemperature = bmp.readTemperature();
    sendData(Btemperature, "Temperature");
}

void checkPressure()
{
    //檢查是否已經超過間隔時間
    //是的話，就執行動作並且記錄更新時間
    //millis()是擷取現在時間的函數
    if (millis()-previousTimePressure>interval)
    {
     clientPrintPressure();
     previousTimePressure=millis(); 
    }
}

void clientPrintPressure()
{   
    Btemperature = bmp.readTemperature();
    Bpressure = bmp.readPressure();  
    sendData(Bpressure, "Pressure");
}

void sendData(long value, String channelId)
  {
  Serial.println("Connecting to WebSite");
  while (0 == client.connect(SITE_URL, 80))
  {
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }

//Client posts DEVICE_ID to MCS.
//Replace ******** to be your DEVICE_ID 
//(Imporatant) Please do not add excessive spacescontent.
  client.println("POST /mcs/v2/devices/D1WPYX1T/datapoints.csv HTTP/1.1");

//The below syntax is an alternative method to execute posting data to MCS. But, sometimes it is failed to be. 
//===========================================
//Serial.println("send HTTP GET request");
//String action = "POST /mcs/v2/devices/";
//action +=DEVICE_ID;                     
//action +="datapoints.csv HTTP/1.1";
//client.println(action);
//===========================================
//The format must present as same as original one.*/

//Client posts data to MCS.
//This syntax is same as pervious one's alterntive method.
  String data = channelId + ",,";
  data += value;
  Serial.println("send Data"  + data);
  int dataLength = data.length();

//Post the other datas to MCS.
  client.println("Host: api.mediatek.com");
  client.print("deviceKey: ");
  client.println(DEVICE_KEY);
  client.print("Content-Length: ");
  client.println(dataLength);
  client.println("Content-Type: text/csv");
  client.println("Connection: close");
  client.println();
  client.println(data);
  
//Waiting for server response
  Serial.println("waiting HTTP response:");
  while (!client.available())
  {
    delay(100);
  }
  
//Make sure we are connected, and dump the response content to Serial
  while (client)
  {
       int v = client.read();
  
   if (v != -1)
    {
      Serial.print((char)v);
    }
    else
    {
      Serial.println("no more content, disconnect");
      client.stop();
      /*此段程式用於測試上傳MCS後，是否有確實上傳資料於MCS之中；此為一個無限回圈，當資料傳完進入此段回圈不再往下進行。
      while (1)
      {
      delay(1);
      }
      ///////////////////////////////////////////////////////////////////////////////////////////////*/
    }
  }
  
//Disconnection messenge from MCS server site to client.   
  if (!disconnectedMsg)
  {
    Serial.println("disconnected by server");
    disconnectedMsg = true;
  }
  delay(500);
} 
