#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Inspirted by https://www.instructables.com/id/Remote-Temperature-Monitoring-Using-MQTT-and-ESP82/
// https://github.com/PaulStoffregen/OneWire
// https://github.com/milesburton/Arduino-Temperature-Control-Library
// https://github.com/knolleary/pubsubclient

// Data wire GPIO2 Pin D4 on Wemo D1 Mini
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// WIFI SSID and password
const char* ssid = "BUBBLE2";
const char* password = "lhouse00";

// This device hostname
const char* thisHost = "WD1_2";

// Address of MQTT Broker
const char* mqtt_server = "192.168.1.20";

// Topic to publish to
const char* mqtt_publishTopic = "bubble/masterbedroom/temperature";

// LED indicator when publishing
bool LEDIndicator = false;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
float temp = 0;

void setup_wifi() {
  delay(10);
  // Connect to a WiFi network

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
  
  // client.subscribe("loungeac");
    
  // LED
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // LED Off  
  
  sensors.begin();

}

void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 
  Serial.println();
  Serial.println("-----------------------");
 
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 10000) {

    if(LEDIndicator){
      digitalWrite(BUILTIN_LED, LOW); // LED On
      }

    lastMsg = now;
    sensors.setResolution(12);
    sensors.requestTemperatures(); // Send the command to get temperatures
    temp = sensors.getTempCByIndex(0);
    Serial.print("Temp: ");
    Serial.println(temp);

    Serial.print("Publishing... ");   
    
    if((temp > -20) && (temp <60))
      {
      client.publish(mqtt_publishTopic, String(temp,1).c_str(),true);
      }

    Serial.println("Done");
    digitalWrite(BUILTIN_LED, HIGH); // LED Off
    // client.subscribe("loungeac");
  }
}
