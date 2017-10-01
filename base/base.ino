#include <SPI.h>
#include <Ethernet.h>

#include <JeeLib.h>
#include <UTFT.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//char server[] = "weatherstation.wunderground.com";
char server[] = "www.wunderground.com";


#define myNodeID 20
#define network 210
#define freq RF12_433MHZ
#define DEBUG 0

IPAddress ip(192, 168, 0, 177);
EthernetClient client;

extern uint8_t SmallFont[];

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
    UTFT LCD;
    unsigned long seconds;
    boolean initialized;
    
  public:
    Base_Elem(int);
    void receiveData();
    void sendOverInternet();
    void printValues();
    void displayValues();
    boolean isInitialized() const;
    void Init();
};

String DisplayAddress(IPAddress address)
{
 return String(address[0]) + "." + 
        String(address[1]) + "." + 
        String(address[2]) + "." + 
        String(address[3]);
}

Base_Elem::Base_Elem(int extNodeID)
{
  initialized = false;
  seconds = 0;
  this->extNodeID = extNodeID;
  m.temp = 0;
  m.alt = 0;
  m.pres = 0;
  m.hum = 0;

  rf12_initialize(myNodeID,freq,network);
  LCD = UTFT(9, 3, 4, 6, 8, 9);
  LCD.InitLCD(PORTRAIT);
  LCD.setFont(SmallFont);
  LCD.clrScr();
  LCD.setContrast(50);
  LCD.fillScr(0, 0, 0);

  LCD.setColor(255, 0, 0);
  LCD.setBackColor(0, 0, 0);
  LCD.print(String("RF12B module "), 0, 5, 0);
  LCD.print(String("initialized"), 10, 15, 0);

  LCD.setColor(0, 255, 255);
  LCD.print("Establishing", 10, 40, 0);
  LCD.print("Ethernet", 15, 50, 0);
  LCD.print("connection", 12, 60,0);
  
  Serial.println("Initialized connection on RFM12B");
  if (Ethernet.begin(mac) == 0) {
    #if DEBUG == 1
      Serial.println(F("Failed to configure Ethernet using DHCP"));
    #endif
    Ethernet.begin(mac, ip);
  }
  delay(2000);
  
  #if DEBUG == 1
    Serial.println(F("connecting..."));
  #endif
  client.connect(server, 80);
  LCD.print(DisplayAddress(Ethernet.localIP()), 4, 70,0);
  
  delay(2000);
  LCD.setColor(255, 0, 0);
  LCD.setBackColor(0, 0, 0);
  LCD.print(String("Waiting for data..."), 0, 90, 0);
  
  LCD.setColor(0, 255, 255);
  LCD.print("Please turn on", 5, 100, 0);
  LCD.print("the ext module", 7, 110,0);
}

void Base_Elem::Init()
{
  if (initialized) return;
  initialized = true;
  LCD.clrScr();
  LCD.setContrast(50);
  LCD.fillScr(0, 0, 0);
}

boolean Base_Elem::isInitialized() const
{
  return initialized;
}

void Base_Elem::receiveData()
{
  #if DEBUG == 1
    Serial.println(F("Waiting for radio data"));
  #endif
  
  bool flag = false;
  while(!flag)
  {
    if(seconds != 0)
    {
     LCD.setColor(0, 255, 0);
     LCD.print(String("Last: ") + String(((millis() - seconds) / 1000L) / 60) + String("m ago"), 0, 45, 0); 
    }
    if(rf12_recvDone())
    {
      Serial.println("Received");
      if(rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0)
      {
        byte node_id = (rf12_hdr & 0x1F);
        if (node_id == extNodeID)
        {
          seconds = millis();
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
  
  #if DEBUG == 1
    Serial.println(F("connected"));
  #endif
  // Make a HTTP request:
  String url = "/weatherstation/updateweatherstation.php?ID=IBUCHARE82&PASSWORD=iuiqtmdp&dateutc=now&action=updateraw";
  url += "&humidity=" + String(m.hum);
  url += "&tempf=" + String(9/5.0 * m.temp + 32.0);
  url += "&baromin=" + String(m.pres * 29.92 / 101325);
  url += "&dewptf=" + String(9/5.0 * (m.temp -  (100.0 - m.hum) /5.0) + 32.0);
  #if DEBUG == 1
    Serial.println(url);
  #endif
  client.println("GET " + url + " HTTP/1.1");
  client.println("Host: " + String(server));
  client.println("Connection: close");
  client.println();
 

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

void drawTemp(UTFT LCD, float temp, int x, int y) {
  LCD.setColor(0 , 255, 255);
  LCD.drawCircle(x, y, 4);
  LCD.fillCircle(x, y, 4);
  LCD.setColor(0 , 0, 0);
  LCD.fillRect(x-2, y-2, x+2, y-35);
  LCD.setColor(0 , 255, 255);
  LCD.drawRect(x-2, y-2, x+2, y-35);
  LCD.printNumI((int)temp, x-30, y-35);
  LCD.drawCircle(x-13, y-35, 2);
  LCD.print("C", x-10, y-35);
  
  LCD.fillRect(x-2, y-2, x+2, y - 33 * temp/50);
}

void Base_Elem::displayValues() {
  
  LCD.setColor(0, 255, 0);
  LCD.print(String("Preasure: "), 0, 15, 0);
  LCD.print(String((long)(m.pres/100.0F)) + String("hPa"), 70, 15, 0);
  LCD.print(String("Humidity: ") + String((long)m.hum) + String("%"), 0, 30, 0);
  LCD.setColor(0, 0, 255);
  drawTemp(LCD, m.temp, 64, 130);
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
  if (!base->isInitialized())
  {
    base->Init();
  }
  base->sendOverInternet();
  base->displayValues();
  delay(500);
}

