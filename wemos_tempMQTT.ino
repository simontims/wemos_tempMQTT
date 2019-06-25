#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Inspired by https://www.instructables.com/id/Remote-Temperature-Monitoring-Using-MQTT-and-ESP82/
// https://github.com/PaulStoffregen/OneWire
// https://github.com/milesburton/Arduino-Temperature-Control-Library
// https://github.com/knolleary/pubsubclient


// SETTINGS //

// Data wire GPIO2 Pin D4 on Wemo D1 Mini
#define ONE_WIRE_BUS 4

// WIFI SSID and password
const char* ssid = "mywifissid";
const char* password = "secret";

// This device hostname
const char* thisHost = "hostname";

// Address of MQTT Broker
const char* mqtt_server = "192.168.1.20";

// Topic to publish to
const char* mqtt_publishTopic = "topic/to/publishTo";

// Set retention
const bool retainMessage = true;

// Wait between publishing (ms)
const int publishWait = 5000;

// LED indicator when publishing
bool LEDIndicator = false;

// END OF SETTINGS //


// Setup a oneWire instance to communicate with OneWire devices 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Setup PubSubClient
WiFiClient espClient;
PubSubClient client(espClient);

// Declare vars
long lastMsg = 0;
float temp = 0;

void setup_wifi() {
  // Short wait before connecting to wifi
  delay(10);

  // Network options - set before Wifi.begin 
  WiFi.hostname(thisHost);  
  // Wifi.config(staticIP, subnet, gateway, DNS);
  
  Serial.println();
  Serial.print("Hostname: ");
  Serial.println(thisHost);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(thisHost)) {
      Serial.println(" connected");
    } else {
      Serial.print(" failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
 
void setup()
{
  Serial.begin(115200);
  setup_wifi(); 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  // To subscribe to any MQTT topics during setup, set them here
  // client.subscribe("topic/to/subscribeto");
    
  // LED Off
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  sensors.begin();
}

// Callback if any messages arrived in subscribed topics
void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 
  Serial.println();
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > publishWait) {
    lastMsg = now;

    // LED on if enabled in settings
    if(LEDIndicator){
      digitalWrite(BUILTIN_LED, LOW);
      }

    sensors.setResolution(12);
    sensors.requestTemperatures();
    temp = sensors.getTempCByIndex(0);
    Serial.print("Temp: ");
    Serial.println(temp);

    Serial.print("Publishing... ");   
    
    if((temp > -20) && (temp < 60))
      {
      client.publish(mqtt_publishTopic, String(temp,1).c_str(), retainMessage);
      }

    Serial.println("Done");

    // Turn LED off at end of MQTT publish
    digitalWrite(BUILTIN_LED, HIGH);

    // To subscribe to any MQTT topics during main, set them here
    // client.subscribe("topic/to/subscribeto");
  }
}
