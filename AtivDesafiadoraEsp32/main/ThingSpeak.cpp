#include "ThingSpeak.h"

// Chave de escrita do ThingSpeak (deve ser gerada em sua conta ThingSpeak)
const char* apikey = "5PEPG6HXN1780M27";

WiFiClient client;        // Cria um objeto WiFiClient necessário para conexões HTTP no ESP8266
HTTPClient http;          // Cria um objeto HTTPClient para enviar requisições HTTP

void subir_thingspeak(float temperatura, float nivel) {
    // Monta a URL para enviar dados ao ThingSpeak
    String url = "http://api.thingspeak.com/update?api_key=";
    url += apikey;                    // Adiciona a chave de API à URL
    url += "&field1=" + String(temperatura); // Adiciona o valor de temperatura ao campo "field1"
    url += "&field2=" + String(nivel); // Adiciona o valor de temperatura ao campo "field2"

    // Imprime a URL gerada no monitor serial (útil para depuração)
    Serial.println("Enviando: " + url);
    Serial.println("Temperatura: " + String(temperatura));
    Serial.println("Nível: " + String(nivel));

    // Inicia a requisição HTTP usando o objeto "client" e a URL gerada
    http.begin(client, url);

    // Executa a requisição HTTP GET e armazena o código de resposta
    int httpResponseCode = http.GET();

    // Verifica se a requisição foi bem-sucedida (código 200 indica sucesso)
    if (httpResponseCode > 0) {
        Serial.println("Resposta do servidor: " + String(httpResponseCode));
      } else {
        // Em caso de erro, imprime a mensagem correspondente
        Serial.println("Erro ao enviar: " + http.errorToString(httpResponseCode));
      }
  
      // Finaliza a conexão HTTP (libera recursos)
      http.end();
}