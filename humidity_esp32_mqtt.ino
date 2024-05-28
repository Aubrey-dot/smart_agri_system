#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <DHT.h>

String URL = "http://192.168.1.159/dht11_project/test_data.php";

const char* ssid     = "Au-B";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
#define Type DHT11
int sensorPin = 4;

DHT HT(sensorPin, Type);
float temperature;
float humidity;

void setup_wifi(){
  delay(10);
  Serial.println();
  Serial.print("Connecting");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);

  while (WiFi.status() != WL_CONNECTED){
    delay(50);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("Wifi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length){
  Serial.print("message arrived [");
  Serial.print(topic);
  Serial.print("]");
  for (int i = 0;i < length; i++){
    Serial.print((char)payload[i]);
  }
  Serial.println();

   // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(2, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(2, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect(){
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
      // Once connected, publish an announcement...
      client.publish("iotfrontier/mqtt", "iotfrontier");
      // ... and resubscribe
      client.subscribe("iotfrontier/mqtt");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void  setup()
{
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  HT.begin();
  delay(500);
 
}

void loop()
{

  if (!client.connected()){
    reconnect();
  }
  client.loop();
  
  unsigned long now = millis();
  if (now - lastMsg > 2000){
    lastMsg = now;
    humidity = HT.readHumidity();
    temperature = HT.readTemperature();

  Serial.print("Humidity: ");
  Serial.print(humidity);
  String hum = String(humidity);
  client.publish("iotfrontier/humidity", hum.c_str());

  Serial.print(" Temperature: ");
  Serial.println(temperature);
  String temp = String (temperature);
  client.publish("iotfrontier/temperature", temp.c_str());
  
  }
  
  

  
}
