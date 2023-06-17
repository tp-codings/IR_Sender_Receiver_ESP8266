#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <IRremote.hpp>

//topicnames und Pinbelegung
#define inTopic "espRemote/inTopic"
#define outTopic "espRemote/outTopic"
#define  SEND_PIN 5

//Keys
const char* wifi_ssid;
const char* wifi_password;
const char* mqtt_server;
const char* mqtt_user;         
const char* mqtt_password;
const char* ESPHostname;

//IR-Daten für Beamer und FireTV
uint32_t rawDataBeamer = 0xBF400681;
uint16_t addressBeamer= 0x681;
uint8_t commandBeamer = 0x40;
uint8_t repeatsBeamer = 5;

uint32_t rawDataFireTV = 0xFF000033;
uint16_t addressFireTV= 0x33;
uint8_t commandFireTV = 0x0;
uint8_t repeatsFireTV = 5;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup(){
  Serial.begin(115200);
  initKeys();
  initOTA();
  initWifi();

  //Konfiguration des Sende-Pins und der LED
  pinMode(SEND_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  IrSender.begin(SEND_PIN);

  //Konfiguration des MQTT-Clients
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop(){

  //Verbindungsaufbau zum MQTT-Server
  if (!client.connected()) {
    reconnect();
  }

  //Ausschalten der LED
  digitalWrite(LED_BUILTIN, HIGH);

  //Verarbeitung der MQTT-Nachrichten
  client.loop();

  //Verwaltung der OTA-Aktualisierung
  ArduinoOTA.handle(); 
  delay(500);
}

//Initialisiere Keys
void initKeys(){
  wifi_ssid = "";
  wifi_password = "";
  mqtt_server = "";
  mqtt_user = "";         
  mqtt_password = "";
  ESPHostname = "";
}

void initOTA(){
  //Anonyme Funktionen für Eventualitäten
  ArduinoOTA.setHostname(ESPHostname);
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  // ArduinoOTA.setPassword("admin");

  //Aktivierung der Überwachung von Aktualisierungsanfragen
  ArduinoOTA.begin();
}

void initWifi() {
  delay(10);
  
  //Verbindungsaufbau WLAN
  Serial.println();
  Serial.print("Verbindungsaufbau WLAN");
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WLAN verbunden");
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  //Bei Befehl '1' wird das entsprechende IR-Signal an Beamer und FireTV gesendet
  if ((char)payload[0] == '1') {
    IrSender.sendNEC(addressBeamer, commandBeamer, repeatsBeamer);
    IrSender.sendNEC(addressFireTV, commandFireTV, repeatsFireTV);
    client.publish(outTopic, "gesendet...");
  } else {
    client.publish(outTopic, "kein Befehl bekannt");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Verbindungsaufbau MQTT...");

    // Zufälliger Client
    String clientId = "ESP01-";
    clientId += String(random(0xffff), HEX);
    
    //Verbindungsaufbau MQTT
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.publish(outTopic, ESPHostname);
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}
