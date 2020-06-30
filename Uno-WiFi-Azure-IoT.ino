#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include <WiFiEspServer.h>
#include <WiFiEspUdp.h>

#include <AzureIoTHubMQTTClient.h>

#include <Base64.h>
#include <MQTT.h>
#include <NtpClientLib.h>
#include <PubSubClient.h>
#include <PubSubClient_JSON.h>
#include <sha256.h>
#include <Utils.h>




#include <SoftwareSerial.h>

#define RX 10
#define TX 11
SoftwareSerial esp8266(RX,TX);

// Azure IoT Hub Settings --> CHANGE THESE
// #define IOTHUB_HOSTNAME         "IoTHubCookeBook-dlau504.azure-devices.net"
// #define DEVICE_ID               "esp32-dlau-001"
// #define DEVICE_KEY              "xDpbufydDEpS8bsrPuYyqmiIFXJZz5M+9I2E8XJZRik=" //Primary key of the device

#define USE_BMP180              0 //Set this to 0 if you don't have the sensor and generate random sensor value to publish

WiFiClientSecure tlsClient;
AzureIoTHubMQTTClient client(tlsClient, IOTHUB_HOSTNAME, DEVICE_ID, DEVICE_KEY);


void connectToIoTHub(); // <- predefine connectToIoTHub() for setup()
void onMessageCallback(const MQTT::Publish& msg);

void onClientEvent(const AzureIoTHubMQTTClient::AzureIoTHubMQTTClientEvent event) {
    if (event == AzureIoTHubMQTTClient::AzureIoTHubMQTTClientEventConnected) {

        Serial.println("Connected to Azure IoT Hub");

        //Add the callback to process cloud-to-device message/command
        client.onMessage(onMessageCallback);
    }
}

void onActivateRelayCommand(String cmdName, JsonVariant jsonValue) {

    //Parse cloud-to-device message JSON. In this example, I send the command message with following format:
    //{"Name":"ActivateRelay","Parameters":{"Activated":0}}

    JsonObject& jsonObject = jsonValue.as<JsonObject>();
    if (jsonObject.containsKey("Parameters")) {
        auto params = jsonValue["Parameters"];
        auto isAct = (params["Activated"]);
        if (isAct) {
            Serial.println("Activated true");
            digitalWrite(LED_PIN, HIGH); //visualize relay activation with the LED
        }
        else {
            Serial.println("Activated false");
            digitalWrite(LED_PIN, LOW);
        }
    }
}

#define INTERVAL 10000
#define MESSAGE_MAX_LEN 256
// Please input the SSID and password of WiFi
const char* ssid     = "TP-LINK_E935";
const char* password = "92724112";

// static const char* connectionString = "HostName=IoTHubCookeBook-dlau504.azure-devices.net;DeviceId=esp32-dlau-001;SharedAccessKey=xDpbufydDEpS8bsrPuYyqmiIFXJZz5M+9I2E8XJZRik=";
const char *messageData = "{\"messageId\":%d, \"Temperature\":%f, \"Humidity\":%f}";
static bool hasIoTHub = false;
static bool hasWifi = false;
int messageCount = 1;
static bool messageSending = true;
static uint64_t send_interval_ms;
/*
static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result)
{
  if (result == IOTHUB_CLIENT_CONFIRMATION_OK)
  {
    Serial.println("Send Confirmation Callback finished.");
  }
}

static void MessageCallback(const char* payLoad, int size)
{
  Serial.println("Message callback:");
  Serial.println(payLoad);
}

static void DeviceTwinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char *payLoad, int size)
{
  char *temp = (char *)malloc(size + 1);
  if (temp == NULL)
  {
    return;
  }
  memcpy(temp, payLoad, size);
  temp[size] = '\0';
  // Display Twin message.
  Serial.println(temp);
  free(temp);
}

static int  DeviceMethodCallback(const char *methodName, const unsigned char *payload, int size, unsigned char **response, int *response_size)
{
  LogInfo("Try to invoke method %s", methodName);
  const char *responseMessage = "\"Successfully invoke device method\"";
  int result = 200;

  if (strcmp(methodName, "start") == 0)
  {
    LogInfo("Start sending temperature and humidity data");
    messageSending = true;
  }
  else if (strcmp(methodName, "stop") == 0)
  {
    LogInfo("Stop sending temperature and humidity data");
    messageSending = false;
  }
  else
  {
    LogInfo("No method %s found", methodName);
    responseMessage = "\"No method found\"";
    result = 404;
  }

  *response_size = strlen(responseMessage) + 1;
  *response = (unsigned char *)strdup(responseMessage);

  return result;
}

*/

void setup() {
  Serial.begin(9600);
  esp8266.begin(9600);

  // initialize ESP8266 module
  Serial.println("ESP8266 Device");
  Serial.println("Initializing...");
  
  WiFi.init(&esp8266); 
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }
  

  Serial.println(" > WiFi");
  Serial.println("Starting connecting WiFi.");

  delay(10);
  WiFi.mode(WIFI_AP);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    hasWifi = false;
  }
  hasWifi = true;
  
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(" > IoT Hub");

  connectToIoTHub();

  client.onEvent(onClientEvent);

  client.onCloudCommand("ActivateRelay", onActivateRelayCommand);

}

void onMessageCallback(const MQTT::Publish& msg) {

    //Handle Cloud to Device message by yourself.

//    if (msg.payload_len() == 0) {
//        return;
//    }

//    Serial.println(msg.payload_string());
}

void connectToIoTHub() {

    Serial.print("\nBeginning Azure IoT Hub Client... ");
    if (client.begin()) {
        Serial.println("OK");
    } else {
        Serial.println("Could not connect to MQTT");
    }
}


void readSensor(float *temp, float *press) {
    //If you don't have the sensor
    *temp = 20 + (rand() % 10 + 2);
    *press = 90 + (rand() % 8 + 2);
}

void loop() {
   client.run();

    if (client.connected()) {

        // Publish a message roughly every 3 second. Only after time is retrieved and set properly.
        if(millis() - lastMillis > 3000 && timeStatus() != timeNotSet) {
            lastMillis = millis();

            //Read the actual temperature from sensor
            float temp, press;
            readSensor(&temp, &press);

            //Get current timestamp, using Time lib
            time_t currentTime = now();

            // You can do this to publish payload to IoT Hub
            /*
            String payload = "{\"DeviceId\":\"" + String(DEVICE_ID) + "\", \"MTemperature\":" + String(temp) + ", \"EventTime\":" + String(currentTime) + "}";
            Serial.println(payload);

            //client.publish(MQTT::Publish("devices/" + String(DEVICE_ID) + "/messages/events/", payload).set_qos(1));
            client.sendEvent(payload);
            */

            //Or instead, use this more convenient way
            AzureIoTHubMQTTClient::KeyValueMap keyVal = {{"MTemperature", temp}, {"MPressure", press}, {"DeviceId", DEVICE_ID}, {"EventTime", currentTime}};
            client.sendEventWithKeyVal(keyVal);
        }
    }
    else {

    }

    delay(10); // <- fixes some issues with WiFi stability
}
