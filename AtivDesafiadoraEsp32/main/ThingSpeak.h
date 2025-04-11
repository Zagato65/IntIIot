#ifndef THINGSPEAK_H
#define THINGSPEAK_H

#include <WiFi.h> // Biblioteca para gerenciar a conexão Wi-Fi
#include <HTTPClient.h> // Biblioteca para realizar requisições HTTP

void subir_thingspeak(float temperatura, float nivel);

#endif