#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H
#define MAX_TOPICS 4

#include <WiFi.h>
#include <PubSubClient.h>

// Estrutura para mapear tópicos a variáveis
struct TopicMap {
  const char* topic;  // Nome do tópico
  float* variable;    // Ponteiro para a variável (float neste caso)
};

extern TopicMap topicMappings[MAX_TOPICS];
extern int numMappings;
void mqtt_map_topics(const TopicMap* mappings, int count);

void setup_wifi(const char* ssid, const char* password);
void mqtt_setup(const char* broker, uint16_t port = 1883);
void mqtt_loop();
void mqtt_publish(const char* topic, const char* message);
void mqtt_set_callback(void (*callback)(char*, byte*, unsigned int));

#endif