#include <M5Stack.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include <OneWire.h>
#include <DallasTemperature.h>
 
const char* ssid = "";
const char* password =  "";
const char* mqttServer = "";
const char* mqttClient = "xbox_telematics";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";

byte* temperature;
int temperatureLength;
 
WiFiClient espClient;
PubSubClient client(espClient);

// GPIO where the DS18B20 is connected to
const int oneWireBus = 3;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
 
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  temperature = payload;
  temperatureLength = length;
}
 
void setup() {
 
  Serial.begin(115200);
  sensors.begin();

  // Initialize the M5Stack object
  M5.begin();

  /*
    Power chip connected to gpio21, gpio22, I2C device
    Set battery charging voltage and current
    If used battery, please call this function in your project
  */
  M5.Power.begin();
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(3);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect(mqttClient)) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
 
//  client.subscribe("xbox/echaust/c");
 
}
 
void loop() {
  sensors.requestTemperatures(); 
  float exhaustTemperatureC = sensors.getTempCByIndex(0);
  float exhaustTemperatureF = sensors.getTempFByIndex(0);

  float ambientTemperatureC = sensors.getTempCByIndex(1);
  float ambientTemperatureF = sensors.getTempFByIndex(1);
  
  Serial.print(exhaustTemperatureC);
  Serial.println("ºC");
  Serial.print(exhaustTemperatureF);
  Serial.println("ºF");

  Serial.print(ambientTemperatureC);
  Serial.println("ºC");
  Serial.print(ambientTemperatureF);
  Serial.println("ºF");

  char s[5];

  float deltaC = (exhaustTemperatureC - ambientTemperatureC);
  float deltaF = (exhaustTemperatureF - ambientTemperatureF);
  client.publish("xbox/exhaust/c", dtostrf(exhaustTemperatureC, 6, 2, s));
  client.publish("xbox/exhaust/f", dtostrf(exhaustTemperatureF, 6, 2, s));

  client.publish("xbox/ambient/c", dtostrf(ambientTemperatureC, 6, 2, s));
  client.publish("xbox/ambient/f", dtostrf(ambientTemperatureF, 6, 2, s));

  client.publish("xbox/delta/c", dtostrf(deltaC, 6, 2, s));
  client.publish("xbox/delta/f", dtostrf(deltaF, 6, 2, s));


  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("Exhaust");

  M5.Lcd.setCursor(10, 50);

  M5.Lcd.print(exhaustTemperatureC);

  M5.Lcd.setCursor(10, 110);
  M5.Lcd.print("Ambient");

  M5.Lcd.setCursor(10, 150);
  M5.Lcd.print(ambientTemperatureC);

  M5.Lcd.setCursor(180, 70);
  M5.Lcd.println("Delta");

  M5.Lcd.setCursor(180, 110);
  M5.Lcd.println(deltaC);
  
  client.loop();

  delay(5000);
}