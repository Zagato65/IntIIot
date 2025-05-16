#include <OneWire.h> // Inclui a biblioteca OneWire para comunicação com dispositivos 1-Wire,
#include <DallasTemperature.h> // Inclui a biblioteca DallasTemperature para trabalhar com sensores de temperatura Dallas/Maxim (como o DS18B20).

//******************************************************************************
#include <WiFi.h>
#include <PubSubClient.h>
#include <ThingSpeak.h>

// Configurações Wi-Fi
const char* ssid = "MFIoT";    // Substitua pelo seu SSID
const char* password = "12345678"; // Substitua pela senha do Wi-Fi

// Parametros de conexão ao MQTT Broker
const char *mqtt_broker = "192.168.137.1"; // host do broker
//const char* mqttClientName = "ESP01S_client";

const char *topicPubTemp = "GRP5/TEMPERATURA"; // topico para publicacao
const char *topicPubNivel = "GRP5/NIVEL"; // topico para publicacao

const char *topicSubGrp1Temp = "GRP1/TEMP"; // topico para subscricao
const char *topicSubGrp1Nivel = "GRP1/NIVEL"; // topico para subscricao
const char *topicSubGrp2Temp = "GRP3/TEMP"; // topico para subscricao
const char *topicSubGrp2Nivel = "GRP3/NIVEL"; // topico para subscricao

const char *mqtt_username = ""; // Usuario nao necessario no host test.mosquitto.org
const char *mqtt_password = ""; // Senha Usuario nao necessario no host test.mosquitto.org
const int mqtt_port = 1883; // Porta

// Variavel de status de conexão MQTT
bool mqttStatus = 0;
float Temp = 0.0;
float Nivel = 0.0;
String MsqTemp;
String MsqNivel;

float TempPlac1 = 0.0;
float NivelPlac1 = 0.0;
float TempPlac2 = 0.0;
float NivelPlac2 = 0.0;

float TempMedia = 0.0;
float NivelMedio= 0.0;

// Instâncias do WiFi e MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Prototipos de funcoes
void setupWiFi();
bool connectMQTT();
void bombaFluxo(bool estado);
float obterTemperaturaCelsius();
float medirDistanciaCM();
void callback(char *topic, byte * payload, unsigned int length);

//******************************************************************************
//Definições de pino, biblioteca de comunicação oneWire e biblioteca Dallas (do componente)
// Define o pino conectado ao pino de dados do DS18B20.
const int pinoTemperatura = 21; // Define a constante inteira 'pinoTemperatura' com o valor 21 (pino D21).
// Inicializa a biblioteca OneWire para se comunicar com dispositivos 1-Wire no pino definido.
OneWire sensorOneWireBus(pinoTemperatura);

// Inicializa a biblioteca DallasTemperature, passando a instância da biblioteca OneWire para comunicação com o sensor.
DallasTemperature sensors(&sensorOneWireBus);

//---//
// Define conexões e constantes para uso do HC-SR04
// Define o pino do ESP32 conectado ao pino Trig do sensor HC-SR04.
const int trigPin = 5;
// Define o pino do ESP32 conectado ao pino Echo do sensor HC-SR04.
const int echoPin = 18;
// Define a velocidade do som em centímetros por microssegundo.
#define SOUND_SPEED 0.034
// Define o fator de conversão de centímetros para polegadas.
#define CM_TO_INCH 0.393701

// Variável para armazenar a duração do pulso de eco.
long duration;
// Variável para armazenar a distância calculada em centímetros.
float distanceCm;
// Variável para armazenar a distância calculada em polegadas.
float distanceInch;

//---//
// Define constantes para controlar o estado do relé da bomba
// Define a constante booleana 'ON' com o valor 'true'.
const bool ON = true;
// Define a constante booleana 'OFF' com o valor 'false'.
const bool OFF = false;

// Define o pino do ESP32 conectado ao relé da bomba.
const int releBomba = 4; // Define a constante inteira 'releBomba' com o valor 5 (pino D5).

void setup() {
    // Define o pino do relé da bomba como uma saída digital.
    pinMode(releBomba, OUTPUT);
    pinMode(pinoTemperatura, INPUT);
    // Inicializa a comunicação serial com uma taxa de 9600 bauds.
    Serial.begin(9600);
    Serial.println("Inicializando sensor DS18B20..."); // Imprime uma mensagem no monitor serial.
    sensors.begin(); // Inicializa a comunicação com o sensor DS18B20.
    delay(500); // Pausa a execução por 500 milissegundos.
    Serial.print("Número de dispositivos encontrados: ");
    // Imprime uma mensagem no monitor serial.
    Serial.println(sensors.getDeviceCount());
    // Imprime o número de sensores DS18B20 encontrados no barramento 1-Wire.
    Serial.println("Sensor DS18B20 inicializado.");
    // Imprime uma mensagem no monitor serial.

    Serial.println("Inicializando sensor HC-SR04...");
    //Imprime uma mensagem no monitor serial.
    pinMode(trigPin, OUTPUT); // Define o pino Trig do HC-SR04 como uma saída digital.
    pinMode(echoPin, INPUT);// Define o pino Echo do HC-SR04 como uma entrada digital.
    digitalWrite(trigPin, LOW);// Garante que o pino Trig comece em nível LOW.
    Serial.println("Sensor HC-SR04 inicializado.");
    // Imprime uma mensagem no monitor serial.

    // Inicializa WiFi
    setupWiFi();
    // Inicializa MQTT
    mqttStatus = connectMQTT(); // Chama conexao MQTT com broker e retorna status
}

void loop() {
    // Acende o relé da bomba por 500 milissegundos.
    bombaFluxo(ON); // Chama a função 'bombaFluxo' com o estado 'ON' (liga o relé).
    delay(500);// Pausa a execução por 500 milissegundos.

    // Apaga o relé da bomba por 500 milissegundos.
    bombaFluxo(OFF);// Chama a função 'bombaFluxo' com o estado 'OFF' (desliga o relé).

    delay(500);// Pausa a execução por 500 milissegundos.
    // Obtém a temperatura em graus Celsius chamando a função 'obterTemperaturaCelsius'.
    float temperaturaCelsius = obterTemperaturaCelsius();
    Serial.print("Temperatura: ");
    // Imprime a string "Temperatura: " no monitor serial.
    Serial.print(temperaturaCelsius); // Imprime o valor da temperatura no monitor serial.
    Serial.println(" °C"); // Imprime a unidade de temperatura no monitor serial e pula para a próxima linha.

    delay(1000); // Pausa a execução por 1000 milissegundos (1 segundo).
    // Obtém a distância em centímetros chamando a função 'medirDistanciaCM'.
    float distanciaCM = medirDistanciaCM();
    Serial.print("Distância: ");
    // Imprime a string "Distância: " no monitor serial.
    Serial.print(distanciaCM); // Imprime o valor da distância em centímetros no monitor serial.
    Serial.println(" cm");
    // Imprime a unidade de distância no monitor serial e pula para a próxima linha.

    //Estabele conexao com broker MQTT
    static long long pooling = 0; // define intervalo de pooling
    if ( mqttStatus) { // Verifica se houve conexao
    client.loop();
    // Publicar dados de exemplo a cada 10 segundos para economizar memória
    if (millis() > pooling +5000) {
    pooling = millis();
    Temp = random(0,1000)/10;
    Nivel= random(0,3000)/10;
    MsqTemp = String(Temp);
    MsqNivel= String(Nivel);
    client.publish(topicPubTemp, MsqTemp.c_str());
    client.publish(topicPubNivel, MsqNivel.c_str());
    Serial.println("Mensagens publicadas: " );
    }
    }
}

///////////////////////////////////////////////
// Função para controlar o estado do relé da bomba.
void bombaFluxo(bool estado){
    // Se o estado passado para a função for 'true' (ON).
    if (estado)
    {
    // Define o pino do relé da bomba para nível HIGH (ativa o relé, dependendo da fiação).
    digitalWrite(releBomba,HIGH);
    }
    // Se o estado passado para a função não for 'true' (ou seja, 'false' - OFF).
    else{
    // Define o pino do relé da bomba para nível LOW (desativa o relé, dependendo da fiação).
    digitalWrite(releBomba,LOW);
    }
}

/////////////////////////////////////////////
// Função para obter a temperatura em graus Celsius do sensor DS18B20.
float obterTemperaturaCelsius() {
    sensors.requestTemperatures(); // Envia o comando para iniciar a conversão de temperatura em todos os sensores DS18B20 no barramento.
    return sensors.getTempCByIndex(0); // Retorna a temperatura em graus Celsius do primeiro sensor encontrado (indice 0).
    }


// Funcao para medir a distancia em centímetros usando o sensor HC-SR04.
float medirDistanciaCM() {
    // Limpa o pino Trig, garantindo um pulso LOW inicial.
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Define o pino Trig em estado HIGH por 10 microssegundos para gerar o pulso ultrassônico.
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Lê o pino Echo, retornando a duração do pulso HIGH (tempo de viagem da onda sonora) em microssegundos.
    float duration = pulseIn(echoPin, HIGH);

    // Calcula a distância em centímetros usando a fórmula: distância = (tempo * velocidade do som) / 2.
    float distanceCm = duration * SOUND_SPEED/2;

    return distanceCm; // Retorna o valor da distância calculada em centímetros.
}

/////////////////////////////////////////////
// Conectar ao Wi-Fi
void setupWiFi() {
    delay(10);
    Serial.print("Conectando ao Wi-Fi...");
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts > 20) {
    Serial.println("\nFalha ao conectar no Wi-Fi! Reiniciando...");
    ESP.restart();
    }
    }
    Serial.println("\nWi-Fi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

/////////////////////////////////////////////
// Conectar ao broker MQTT
// Implementacao do metodo de conexao com o broker
bool connectMQTT() {
    byte tentativa = 0; // variavel byte que contará o número de tentativas de conexao
    client.setServer(mqtt_broker, mqtt_port); // chama metodo setServer passando url e port do broker
    client.setCallback(callback); // Informa o objeto client qual metodo deve ser chamado quando houver alguma mensagem no topico subscrito.

    do {
        // Define o ID do cliente (a propria placa ESP)
        String client_id = "ESP-"; // Que usa o prefixo ESP-
        client_id += String(WiFi.macAddress());
        // Concatenado com seu respectivo MAC address

        // O if tenta estabelecer a conexao com o broker
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            // Com sucesso da conexao, informa os dados do cliente (a placa)
            Serial.println("Exito na conexao:");
            Serial.printf("Cliente %s conectado ao broker\n", client_id.c_str());
        } else {
            // Informa falha na conexao e aguarda 2 segundos para nova tentativa
            Serial.print("Falha ao conectar: ");
            Serial.print(client.state());
            Serial.println();
            Serial.print("Tentativa: ");
            Serial.println(tentativa);
            delay(2000);
        }
        tentativa++; // Incrementa numero de tentativas
    } while (!client.connected() && tentativa < 5); // Limita número de tentativas

    if (tentativa < 5) {
        // Conexao realizada com sucesso
        // Se subscreve no broker para receber mensagens
        client.subscribe(topicPubTemp);
        client.subscribe(topicPubNivel);
        client.subscribe(topicSubGrp2Temp);
        client.subscribe(topicSubGrp2Nivel);
        return 1; // retorna 1 confirmando sucesso na conexao
    } else {
        // caso contrário avisa falha e retorna 0
        Serial.println("MQTT Não conectado");
        return 0; // informa falha na conexao
    }
}

//////////////////////////////////////////////

// Este metodo eh chamado quando o client identifica nova mensagem no broker
void callback(char *topic, byte * payload, unsigned int length) {
    // char *topic identifica o tóico registrado
    // byte *payload conjunto de bytes que foram publicados
    // int length é o tamanho do vetor de bytes do pyload
    Serial.print("Mensagem chegou no tópico: ");
    Serial.println(topic);

    // Cria uma String a partir do payload
    String mensagemString = "";
    for (int i = 0; i < length; i++) {
        mensagemString += (char)payload[i];
        Serial.print((char)payload[i]);
    }

    Serial.println();
    // Converte a String para um float
    float valorFloat = mensagemString.toFloat();

    // PLACA 01 EXTERNA
    if (strcmp(topic, topicPubTemp) == 0) {
        // Se os tópicos forem iguais, converte o payload para float e atualiza TempPlac1
        TempPlac1 = mensagemString.toFloat();
        Serial.print("Temperatura da Placa 1 atualizada para: ");
        Serial.println(TempPlac1);
    }

    if (strcmp(topic, topicPubNivel) == 0) {
        // Se os tópicos forem iguais, converte o payload para float e atualiza TempPlac1
        NivelPlac1 = mensagemString.toFloat();
        Serial.print("Nivel da Placa 1 atualizada para: ");
        Serial.println(NivelPlac1);
    }

    // PLACA 02 EXTERNA
    if (strcmp(topic, topicSubGrp2Temp) == 0) {
        // Se os tópicos forem iguais, converte o payload para float e atualiza TempPlac1
        TempPlac2 = mensagemString.toFloat();
        Serial.print("Temperatura da Placa 2 atualizada para: ");
        Serial.println(TempPlac1);
    }

    if (strcmp(topic, topicSubGrp2Nivel) == 0) {
        // Se os tópicos forem iguais, converte o payload para float e atualiza TempPlac1
        NivelPlac2 = mensagemString.toFloat();
        Serial.print("Nivel da Placa 2 atualizada para: ");
        Serial.println(TempPlac1);
    }

    Serial.print("Mensagem (Float): ");
    Serial.println(valorFloat);
    Serial.println("---");
}