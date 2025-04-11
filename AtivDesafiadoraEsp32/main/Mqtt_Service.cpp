#include "mqtt_service.h"

WiFiClient espClient;
PubSubClient client(espClient);

// Variáveis globais para armazenar dados
MqttData mqttData[MAX_TOPICS] = {};
int numTopics = 0;

void setup_wifi(const char* ssid, const char* password) {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
  Serial.println(WiFi.localIP());
}

// Callback genérica para atualizar variáveis
void custom_callback(char* topic, byte* payload, unsigned int length) {
  String mensagem;
  for (int i = 0; i < length; i++) mensagem += (char)payload[i];
  float valor = mensagem.toFloat();

  // Atualiza a variável correspondente
  for (int i = 0; i < numMappings; i++) {
    if (strcmp(topic, topicMappings[i].topic) == 0) {
      *topicMappings[i].variable = valor;
      Serial.print("[MQTT] ");
      Serial.print(topic);
      Serial.print(": ");
      Serial.println(valor);
      break;
    }
  }
}

void mqtt_map_topics(const TopicMap* mappings, int count) {
  if (count > MAX_TOPICS) count = MAX_TOPICS;
  for (int i = 0; i < count; i++) {
    topicMappings[i] = mappings[i];
    client.subscribe(mappings[i].topic);
    numMappings++;
  }
}

void mqtt_set_callback(void (*callback)(char*, byte*, unsigned int)) {
  client.setCallback(callback); // Define o callback personalizado
}

void mqtt_subscribe(const char* topic) {
  if (client.subscribe(topic)) {
    Serial.print("Inscrito no tópico: ");
    Serial.println(topic);
  } else {
    Serial.println("Falha na inscrição!");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT... ");
    String clientId = "ESP32";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado!");
       // Reinscreve-se em todos os tópicos
       for (int i = 0; i < numTopics; i++) {
        client.subscribe(mqttData[i].topic);
      }
    } else {
      Serial.print("Falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando de novo em 5 segundos...");
      delay(5000);
    }
  }
}

void mqtt_setup(const char* broker, uint16_t port) {
  client.setServer(broker, port);
}

void mqtt_loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void mqtt_publish(const char* topic, const char* message) {
  client.publish(topic, message);
  Serial.print("Publicou no tópico ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);
}
