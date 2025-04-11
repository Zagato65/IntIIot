#include "Sensor_DS18B20.h"
#include "Mqtt_Service.h"
#include "Converter.h"
#include "Sensor_HC-SCR04.h"
#include "ThingSpeak.h"

const int led = 13;
const char* ssid = "linksys";
const char* password = "";
const char* broker = "broker.hivemq.com";
uint16_t port = 1883;

// Variáveis para armazenar dados
float tempSub1 = 0.0;
float nivelSub1 = 0.0;
float tempSub2 = 0.0;
float nivelSub2 = 0.0;

// Mapeamento de tópicos
const TopicMap mappings[] = {
  {"GRP1\\TEMPERATURA", &tempSub1},
  {"GRP1\\NIVEL", &nivelSub1},
  {"GRP2\\TEMPERATURA", &tempSub2},
  {"GRP2\\NIVEL", &nivelSub2}
};

void setup() {
  pinMode(led, OUTPUT);
  Serial.begin(115200);
  startTemperature();
  setup_wifi(ssid,password);
  mqtt_setup(broker, port);
  mqtt_set_callback(custom_callback); // Usa a callback do service.cpp
  mqtt_map_topics(mappings, 4); // Mapeia 4 tópicos 
}

void loop() {
  mqtt_loop();
  float temp = readTemperature();
  float nivel = lerNivel();

  if (nivel > 25) {
    digitalWrite(led, HIGH);
  } else {
    digitalWrite(led, LOW);
  }

  mqtt_publish("GRP5/TEMPERATURA",floatToStr(temp,2));
  mqtt_publish("GRP5/NIVEL",floatToStr(nivel,2));
  delay(5000);
}
