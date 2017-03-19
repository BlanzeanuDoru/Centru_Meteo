
#include <ESP8266WiFi.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define DEBUG 0

const char * NETWORK = "netis";
const char * PASSWORD = "parolaestegrea";
const char * host = "weatherstation.wunderground.com";

class Centru {
  private:
      bool status;
      Adafruit_BME280 * bme;
      byte sleepTimeS;
      
  public:
      Centru(byte);
      void setup();
      void printValues();
      void sendData();
      void sleep();
};

Centru::Centru(byte sleepTime) {
    bme = new Adafruit_BME280();
    this->sleepTimeS = sleepTime;
}

void Centru::setup() {
    this->status = this->bme->begin();
    if (!status) {
        #if DEBUG == 1
          Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
        #endif
        while (1);
    }

    WiFi.begin(NETWORK, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      #if DEBUG == 1
        Serial.print(F("."));
      #endif
    }

    #if DEBUG == 1
      Serial.println(F(""));
      Serial.println(F("WiFi connected"));  
      Serial.println(F("IP address: "));
      Serial.println(WiFi.localIP());
    #endif
}

void Centru::printValues() {
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
}

void Centru::sendData() {
    //delay(20000);
    #if DEBUG == 1
      Serial.print(F("connecting to "));
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

    String temp = String(9/5.0 * this->bme->readTemperature() + 32.0);
    String pressure = String(this->bme->readPressure() * 29.92 / 101325);
    String dew = String(9/5.0 * (this->bme->readTemperature() -  (100.0 - this->bme->readHumidity()) /5.0) + 32.0);
    
    // We now create a URI for the request
    String url = "/weatherstation/updateweatherstation.php?ID=IBUCHARE82&PASSWORD=iuiqtmdp&dateutc=now&action=updateraw";
    url += "&humidity=" + String(this->bme->readHumidity());
    url += "&tempf=" + temp;
    url += "&baromin=" + pressure;
    url += "&dewptf=" + dew; 
    
    #if DEBUG == 1
      Serial.print(F("Requesting URL: "));
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
          Serial.println(F(">>> Client Timeout !"));
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
      Serial.println(F("closing connection"));
    #endif
}

void Centru::sleep() {
    #if DEBUG == 1
       Serial.println(F("Putting ESP to sleep ..."));
    #endif
    ESP.deepSleep(this->sleepTimeS * 1000000);
    //delay(this->sleepTimeS * 1000);
}

Centru * c = new Centru(900);
unsigned long delayTime;

void setup() {
    #if DEBUG == 1
      Serial.begin(9600);
    #endif
    
    c->setup();
    delayTime = 1000;
    
    #if DEBUG == 1
      Serial.println();
    #endif
}


void loop() { 
    #if DEBUG == 1
      c->printValues();
    #endif
    delay(delayTime);
    c->sendData();
    c->sleep();
}



