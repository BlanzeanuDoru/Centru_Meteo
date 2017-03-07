
#include <ESP8266WiFi.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define DEBUG 1

const char * NETWORK = "netis";
const char * PASSWORD = "parolaestegrea";
const char * host = "weatherstation.wunderground.com";

class Centru {
  private:
      bool status;
      Adafruit_BME280 * bme;
      
  public:
      Centru();
      void setup();
      void printValues();
      void sendData();
};

Centru::Centru() {
    bme = new Adafruit_BME280();
}

void Centru::setup() {
    this->status = this->bme->begin();
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }

    WiFi.begin(NETWORK, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void Centru::printValues() {
    #if DEBUG == 1
      Serial.print(F("Temperature = "));
      Serial.print(bme->readTemperature());
      Serial.println(F(" *C"));
  
      Serial.print(F("Pressure = "));
  
      Serial.print(bme->readPressure() / 100.0F);
      Serial.println(F(" hPa"));
  
      Serial.print(F("Approx. Altitude = "));
      Serial.print(bme->readAltitude(SEALEVELPRESSURE_HPA));
      Serial.println(F(" m"));
  
      Serial.print(F("Humidity = "));
      Serial.print(bme->readHumidity());
      Serial.println(F(" %"));
  
      Serial.println();
    #endif
}

void Centru::sendData() {
    delay(20000);
    #if DEBUG == 1
    Serial.print("connecting to ");
    Serial.println(host);
    #endif
    
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
      #if DEBUG == 1
      Serial.println("connection failed");
      #endif
      return;
    }
    
    // We now create a URI for the request
    String url = "/weatherstation/updateweatherstation.php?dateutc=now&ID=IBUCHARE82&PASSWORD=iuiqtmdp&action=updateraw";
    url += "&indoorhumidity=" + String(this->bme->readHumidity());
    url += "&indoortempf=" + String(this->bme->readTemperature());
    #if DEBUG == 1
    Serial.print("Requesting URL: ");
    Serial.println(url);
    #endif
    
    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        #if DEBUG == 1
        Serial.println(">>> Client Timeout !");
        #endif
        client.stop();
        return;
      }
    }
    
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()){
      String line = client.readStringUntil('\r');
      #if DEBUG == 1
      Serial.print(line);
      #endif
    }

    #if DEBUG == 1
    Serial.println();
    Serial.println("closing connection");
    #endif
}

Centru * c = new Centru();
unsigned long delayTime;

void setup() {
    Serial.begin(9600);
    c->setup();
    delayTime = 1000;
    Serial.println();
}


void loop() { 
    c->printValues();
    c->sendData();
    delay(delayTime);
}



