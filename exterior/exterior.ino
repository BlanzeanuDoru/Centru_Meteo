#include <JeeLib.h>
#include <Ports.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define myNodeID 10
#define network 210
#define freq RF12_433MHZ

#define SEALEVELPRESSURE_HPA (1013.25)
#define DEBUG 0

ISR(WDT_vect) { 
  Sleepy::watchdogEvent(); 
}


struct MESAJ
{
  float temp;
  float pres;
  float alt;
  float hum;
};

void printValues(struct MESAJ &m) {
      Serial.print(F("Temperature = "));
      Serial.print(m.temp);
      Serial.println(F(" *C"));
    
      Serial.print(F("Pressure = "));
    
      Serial.print(m.pres / 100.0F);
      Serial.println(F(" hPa"));
    
      Serial.print(F("Approx. Altitude = "));
      Serial.print(m.alt);
      Serial.println(F(" m"));
    
      Serial.print(F("Humidity = "));
      Serial.print(m.hum);
      Serial.println(F(" %"));
    
      Serial.println();
}

const int baseNodeID = 11;
bool start = false;
MESAJ m;
Adafruit_BME280 bme;


void setup()
{
  #if DEBUG == 1
    Serial.begin(9600);
  #endif

  rf12_initialize(myNodeID,freq,network);
  #if DEBUG == 1
    Serial.println(F("Initialized RFM12 connection.."));
  #endif
  bool status = bme.begin();
  if (!status) {
    #if DEBUG == 1
      Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
    #endif
    while(1);
  }
  
  Sleepy::loseSomeTime(1000);
}

void loop()
{
  m.temp = bme.readTemperature();
  m.pres = bme.readPressure();
  m.alt = bme.readAltitude(SEALEVELPRESSURE_HPA);
  m.hum = bme.readHumidity();

  int i = 0;
  while (!rf12_canSend() && i++ < 10)
  {
    rf12_recvDone();
  }
  //rf12_sendStart(0, &m, sizeof(m));
  rf12_sendNow(0, &m, sizeof(m));
  rf12_sendWait(0);
 
 for(int i=0; i<10; i++) {
    rf12_sleep(RF12_SLEEP);
    Sleepy::loseSomeTime(60000);
    rf12_sleep(RF12_WAKEUP);
 }
  
 
  
}

