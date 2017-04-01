
#include <SPI.h>
#include <Ethernet.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define SEALEVELPRESSURE_HPA (1013.25)

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "weatherstation.wunderground.com";

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);
EthernetClient client;

Adafruit_BME280 bme;

void printValues() {
    Serial.print(F("Temperature = "));
    Serial.print(bme.readTemperature());
    Serial.println(F(" *C"));
  
    Serial.print(F("Pressure = "));
  
    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(F(" hPa"));
  
    Serial.print(F("Approx. Altitude = "));
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(F(" m"));
  
    Serial.print(F("Humidity = "));
    Serial.print(bme.readHumidity());
    Serial.println(F(" %"));
  
    Serial.println();
}

void setup() {
  Serial.begin(9600);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  bool status = bme.begin();
  if (!status) {
    Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
    while(1);
  }
  
  printValues();

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");

  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected");
    // Make a HTTP request:
    String url = "/weatherstation/updateweatherstation.php?ID=IBUCHARE82&PASSWORD=iuiqtmdp&dateutc=now&action=updateraw";
    url += "&humidity=60";
    url += "&tempf=70";
    client.println("GET " + url + " HTTP/1.1");
    client.println("Host: " + String(server));
    client.println("Connection: close");
    client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}

void loop() {
  // if there are incoming bytes available
  // from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
  printValues();
  delay(2000);
}

