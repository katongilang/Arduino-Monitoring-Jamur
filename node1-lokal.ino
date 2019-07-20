#include <ESP8266WiFi.h>
#include <MQTT.h>
#include "DHTesp.h"

#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif

//inisialisasi wifi
const char ssid[] = "MushRoom";
const char pass[] = "abogoboga";
WiFiClient net;

//inisialisasi mqtt
MQTTClient client;
const char* mqttuser = "katon";
const char* mqttpassword = "katon";
const char* deviceId = "nodesensor1";
unsigned long lastMillis = 0;

//inisialisasi pin
DHTesp dht;
int pinSensor=D1;
int pinRelay=D2;

void setup() {
  dht.setup(pinSensor, DHTesp::DHT11);
  pinMode(pinRelay,OUTPUT);
  Serial.begin(115200);
  readWifi();
  Serial.print("connecting to Local MQTT broker...");
  client.begin("192.168.100.6",1883, net);//ip server mqtt dan port
  //client.begin("broker.shiftr.io", net);
  client.onMessage(messageReceived); //callback
  connect();//untuk koneksi ke broker
}

void loop() {
  //baca nilai sensor
  float kelembaban = dht.getHumidity();
  float suhuCelcius = dht.getTemperature();
  float suhuFahrenheit = dht.toFahrenheit(suhuCelcius);

  //cek koneksi ke broker mqtt
  client.loop();
  if(!client.connected()) {
   connect(); //untuk koneksi ke broker mqtt
  }

  //mengirim nilai sensor menuju broker mqtt
  if(millis() - lastMillis > 300000) { //1000=1dtk | 60000x5=300000(5mnt)
   lastMillis = millis();
   client.publish("node1/status/kelembaban", (String)kelembaban);
   client.publish("node1/status/suhuCelcius", (String)suhuCelcius);
   client.publish("node1/status/suhuFahrenheit", (String)suhuFahrenheit);
  }
 }

void connect() {
  while (!client.connect(deviceId, mqttuser, mqttpassword)) {
    Serial.print(".");}
  Serial.println("\nConnected !");
  client.subscribe("node1/status/#");
  client.subscribe("node1/pengabutan/#");
}

void messageReceived(String &topic, String &payload) {  
  Serial.println("incoming: " + topic + " - " + payload);
  
  if(topic=="node1/pengabutan/manual"){
    if(payload=="aktif") {
      digitalWrite(pinRelay, HIGH);
      Serial.println("Pengabutan manual sedang jalan");
      client.publish("node1/pengabutan/manual/relay", "aktif");
    }
    else{
      digitalWrite(pinRelay, LOW);
      Serial.println("Pengabutan manual sedang jalan");
      client.publish("node1/pengabutan/manual/relay", "mati");
    }
   }

   if(topic=="node1/pengabutan/otomatis"){
    if(payload=="aktif") {
      digitalWrite(pinRelay, HIGH);
      Serial.println("Pengabutan otomatis sedang jalan");
      client.publish("node1/pengabutan/otomatis/relay", "aktif");
    }
    else{
      digitalWrite(pinRelay, LOW);
      Serial.println("Pengabutan otomatis mati");
      client.publish("node1/pengabutan/otomatis/relay", "mati");
    }
   }
}

void readWifi(){
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(2000);
}

