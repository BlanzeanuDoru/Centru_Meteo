#include <SPI.h>
#include <Ethernet.h>

#include <JeeLib.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "weatherstation.wunderground.com";

#define myNodeID 20
#define network 210
#define freq RF12_433MHZ
#define DEBUG 1

IPAddress ip(192, 168, 0, 177);
EthernetClient client;

struct MESAJ
{
  float temp;
  float pres;
  float alt;
  float hum;
};

class Base_Elem
{
  private:
    int extNodeID;
    MESAJ m;
  public:
    Base_Elem(int);
    void receiveData();
    void sendOverInternet();
    void printValues();
};

Base_Elem::Base_Elem(int extNodeID)
{
  this->extNodeID = extNodeID;
  m.temp = 0;
  m.alt = 0;
  m.pres = 0;
  m.hum = 0;

  rf12_initialize(myNodeID,freq,network);
  Serial.println("Initialized connection on RFM12B");
  if (Ethernet.begin(mac) == 0) {
    #if DEBUG == 1
      Serial.println(F("Failed to configure Ethernet using DHCP"));
    #endif
    Ethernet.begin(mac, ip);
  }
  
  delay(1000);
  #if DEBUG == 1
    Serial.println(F("connecting..."));
  #endif
  client.connect(server, 80);
}

void Base_Elem::receiveData()
{
  #if DEBUG == 1
    Serial.println(F("Waiting for radio data"));
  #endif
  
  bool flag = false;
  while(!flag)
  {
    
    if(rf12_recvDone())
    {
      if(rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0)
      {
        
        int node_id = (rf12_hdr & 0x1F);
  
        if (node_id == extNodeID)
        {
          
          m = *(MESAJ*) rf12_data;
          flag = true;
        }
      }
    }
  }
  
}

void Base_Elem::sendOverInternet()
{
  if(!client.connected())
  {
      if(!client.connect(server, 80))
      {
        #if DEBUG == 1
          Serial.println(F("connection failed"));
        #endif
        return;
      }
  }
  else
  {
    #if DEBUG == 1
      Serial.println(F("connected"));
    #endif
    // Make a HTTP request:
    String url = "/weatherstation/updateweatherstation.php?ID=IBUCHARE82&PASSWORD=iuiqtmdp&dateutc=now&action=updateraw";
    url += "&humidity=" + String(m.hum);
    url += "&tempf=" + String(9/5.0 * m.temp + 32.0);
    client.println("GET " + url + " HTTP/1.1");
    client.println("Host: " + String(server));
    client.println("Connection: close");
    client.println();
  } 

  while (client.available()) {
    char c = client.read();
    #if DEBUG == 1
      Serial.print(c);
    #endif
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    #if DEBUG == 1
      Serial.println();
      Serial.println(F("disconnecting."));
    #endif
    client.stop();
  }
}


void Base_Elem::printValues() {
    #if DEBUG == 1
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
    #endif
}

Base_Elem * base;
const int extNodeID = 10;

void setup() {
  #if DEBUG == 1
    Serial.begin(9600);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
  #endif

  base = new Base_Elem(extNodeID);
}

void loop() {
  
  base->receiveData();
  delay(1000);
  base->sendOverInternet();
  base->printValues();
  delay(2000);
}

