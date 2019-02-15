

/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <Arduino.h>
#include <Ultrasonic.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <string.h>
#include <cstring>

// Update these with values suitable for your network.

const char* ssid = "Rachel's iPhone";
const char* password = "cmfj1b5fv2kff";
const char* mqtt_server = "infiniteattempts.summerstudio.xyz";

const char* canary_topic = "Canary/Ibis";

#define DHTPIN 5          // What digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22, AM2302, AM2321

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
char payLoad[100];
int serialCounter = 0;
int distance;
float humidity;
float temperature;

uint32_t ChipUid;
String ChipUidString;


DHT dht(DHTPIN, DHTTYPE);
Ultrasonic ultrasonic(D6, D7);

// This function sends Arduino's up time every second to Virtual Pin (5).
// In the app, Widget's reading frequency should be set to PUSH. This means
// that you define how often to send data to Blynk App.

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 5269);
  client.setCallback(callback);

  // Get and store chip ID in string
  ChipUid = ESP.getChipId();
  ChipUidString = String(ChipUid,HEX);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    
    humidity = dht.readHumidity();
    temperature = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
    distance = ultrasonic.read();

    char hmsg[10];
    char tmsg[10];
    dtostrf(humidity,7,3,hmsg);
    dtostrf(temperature,7,3,tmsg);

    if (isnan(humidity) || isnan(temperature)) {
        Serial.println("Failed to read from DHT sensor!");
        return;  
    }
    lastMsg = now;
    ++serialCounter;

    // Serial message to confirm code working
    snprintf (msg, 50, "hello world #%d", serialCounter);
    Serial.print("Publish message: ");
    Serial.println(msg);

    // Compose Payload
    snprintf(payLoad, 100, "%s,H.T.D,%s,%s,%d", ChipUidString.c_str(),hmsg,tmsg,distance);

    client.publish(canary_topic, payLoad);
  }
}
