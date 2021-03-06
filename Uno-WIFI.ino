<<<<<<< HEAD
/*
  ArduinoMqttClient - WiFi Simple Sender

  This example connects to a MQTT broker and publishes a message to
  a topic once a second.

  The circuit:
  - Arduino MKR 1000, MKR 1010 or Uno WiFi Rev.2 board
  - Arduino Uno with External ESP8266.
  Still need to see how to make the ArduinoMqttClient Work for Azure IoT using X.509
  https://create.arduino.cc/projecthub/Arduino_Genuino/securely-connecting-an-arduino-nb-1500-to-azure-iot-hub-af6470
  
  More promising approache is to us PubClient 
  http://www.radupascal.com/2016/04/03/esp8266-arduino-iot-hub
  
  This example code is in the public domain.
*/

#define ARDUINO_ESP
// #define SECRET_SSID TP-LINK_E935
// #define SECRET_PASS 92724112

#include <ArduinoMqttClient.h>


#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
  #include <WiFi101.h>
#elif defined(ARDUINO_ESP8266_ESP12)
  #include <ESP8266WiFi.h>
#elif defined(ARDUINO_ESP)
  #include <SoftwareSerial.h>
  #include <WiFiEsp.h>
  #include <WiFiEspClient.h>
  #include <WiFiEspServer.h>
  #include <WiFiEspUdp.h>
  
  #define RX 10
  #define TX 11
  SoftwareSerial esp8266(RX,TX);

  // Azure IoT Hub Settings --> CHANGE THESE
  #define IOTHUB_HOSTNAME         "IoTHubCookeBook-dlau504.azure-devices.net"
  #define DEVICE_ID               "esp32-dlau-001"
  #define DEVICE_KEY              "xDpbufydDEpS8bsrPuYyqmiIFXJZz5M+9I2E8XJZRik=" //Primary key of the device

#endif

// #include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "TP-LINK_E935";        // your network SSID (name)
char pass[] = "92724112";    // your network password (use for WPA, or use as key for WEP)

// To connect with SSL/TLS:
// 1) Change WiFiClient to WiFiSSLClient.
// 2) Change port value from 1883 to 8883.
// 3) Change broker value to a server with a known SSL/TLS root certificate 
//    flashed in the WiFi module.

#if defined(ARDUINO_ESP)
// WiFiClientSecure wifiClient;
WiFiEspClient wifiClient;
// AzureIoTHubMQTTClient mqttclient(wifiClient, IOTHUB_HOSTNAME, DEVICE_ID, DEVICE_KEY);
MqttClient mqttClient(wifiClient);
#else
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
#endif

const char broker[] = "IoTHubCookeBook-dlau504.azure-devices.net";
int        port     = 1883;
const char topic[]  = "arduino/simple";

const long interval = 1000;
unsigned long previousMillis = 0;

int count = 0;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

    // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);

#if defined(ARDUINO_ESP)
  esp8266.begin(9600);
  while(!esp8266) ;
=======
#include <SoftwareSerial.h>

#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include <WiFiEspServer.h>
#include <WiFiEspUdp.h>

#define RX 10
#define TX 11
SoftwareSerial esp8266(RX,TX);

void printMacAddress() {
  // the MAC address of your Wifi shield
  byte mac[6];                    

  // print your MAC address:
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  Serial.print(mac[5],HEX);
  Serial.print(":");
  Serial.print(mac[4],HEX);
  Serial.print(":");
  Serial.print(mac[3],HEX);
  Serial.print(":");
  Serial.print(mac[2],HEX);
  Serial.print(":");
  Serial.print(mac[1],HEX);
  Serial.print(":");
  Serial.println(mac[0],HEX);
}

void listNetworks() {
  // scan for nearby networks:
  Serial.println("** Scan Networks **");
  byte numSsid = WiFi.scanNetworks();

  // print the list of networks seen:
  Serial.print("number of available networks:");
  Serial.println(numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet<numSsid; thisNet++) {
    Serial.print(thisNet);
    Serial.print(") ");
    Serial.print(WiFi.SSID(thisNet));
    Serial.print("\tSignal: ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.print(" dBm");
    Serial.print("\tEncryption: ");
    Serial.println(WiFi.encryptionType(thisNet));
  }
}

void setup() {
  // initialize serial and wait for the port to open:

  Serial.begin(9600);
  esp8266.begin(9600);
  while(!esp8266) ;

>>>>>>> c43963577dfe108ba30a4d9f697511b769584fd8
    // initialize ESP module
  WiFi.init(&esp8266);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }
<<<<<<< HEAD
#endif

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }


  Serial.println("You're connected to the network");
  Serial.println();

  // You can provide a unique client ID, if not set the library uses Arduino-millis()
  // Each client must have a unique client ID
  // mqttClient.setId("clientId");

  // You can provide a username and password for authentication
  // mqttClient.setUsernamePassword("username", "password");

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
}

void loop() {
  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();

  // avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;

    Serial.print("Sending message to topic: ");
    Serial.println(topic);
    Serial.print("hello ");
    Serial.println(count);

    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(topic);
    mqttClient.print("hello ");
    mqttClient.print(count);
    mqttClient.endMessage();

    Serial.println();

    count++;
  }
=======

  // attempt to connect using WEP encryption:
  Serial.println("Initializing Wifi...");
  printMacAddress();

  // scan for existing networks:
  Serial.println("Scanning available networks...");
  listNetworks();

}

void loop() {
  delay(10000);
  // scan for existing networks:
  Serial.println("Scanning available networks...");
  listNetworks();

>>>>>>> c43963577dfe108ba30a4d9f697511b769584fd8
}
