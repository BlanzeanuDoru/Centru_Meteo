#include <JeeLib.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define myNodeID 10
#define network 210
#define freq RF12_433MHZ

#define SEALEVELPRESSURE_HPA (1013.25)
#define DEBUG 1

struct MESAJ
{
  float temp;
  float pres;
  float alt;
  float hum;
};

class Ext_Elem
{
  private:
    Adafruit_BME280 * bme;
    MESAJ m;
    int baseNodeId;
  public:
    Ext_Elem(int);
    ~Ext_Elem();
    void aquireData();
    void sendData();
    void printValues();
};

Ext_Elem::Ext_Elem(int baseNodeId)
{
  this->baseNodeId = baseNodeId;
  this->bme = new Adafruit_BME280();
  m.temp = 0;
  m.alt = 0;
  m.pres = 0;
  m.hum = 0;

  rf12_initialize(myNodeID,freq,network);
  Serial.println(F("Initialized RFM12 connection.."));
  bool status = bme->begin();
  if (!status) {
    #if DEBUG == 1
      Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
    #endif
    while(1);
  }
}

Ext_Elem::~Ext_Elem()
{
  delete bme;
}

void Ext_Elem::aquireData()
{
  m.temp = bme->readTemperature();
  m.pres = bme->readPressure();
  m.alt = bme->readAltitude(SEALEVELPRESSURE_HPA);
  m.hum = bme->readHumidity();
}

void Ext_Elem::sendData()
{
  int i = 0;
  while (!rf12_canSend() && i++ < 10)
  {
    rf12_recvDone();
  }
  Serial.print(String("Packet size: ") + String(sizeof(m)) + "\n");
  Serial.println(String("rf12_crc = ") + String(rf12_crc));
  Serial.println(String("rf12_hdr = ") + String(rf12_hdr));
  Serial.println(String("rf12_hdr & RF12_HDR_CTL= ") + String(rf12_hdr & RF12_HDR_CTL));
  rf12_sendStart(0, &m, sizeof(m));
}

void Ext_Elem::printValues() {
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

Ext_Elem * myelem;
const int baseNodeID = 11;

void setup()
{
  myelem = new Ext_Elem(baseNodeID);
  #if DEBUG == 1
    Serial.begin(9600);
  #endif 
}

void loop()
{
  myelem->aquireData();
  myelem->sendData();
  myelem->printValues();
  delay(5000);
}

