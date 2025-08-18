/*
#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "Nome_Rede";
const char* password = "Senha_Rede";

WiFiUDP udp;
const char* udpAddress = "255.255.255.255";  // broadcast
const int udpPort = 12345;

unsigned long lastSend = 0;
const int sendIntervalMs = 100;  // 10 Hz

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("Conectando ao WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" conectado!");

  udp.begin(udpPort);  // Pode ser qualquer porta local
}

void loop() {
  unsigned long now = millis();
  if (now - lastSend >= sendIntervalMs) {
    lastSend = now;

    String msg = "ESP32 " + String(now);
    udp.beginPacket(udpAddress, udpPort);
    udp.write((const uint8_t *)msg.c_str(), msg.length());
    udp.endPacket();

    Serial.println("Enviado: " + msg);
  }
}
*/

#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiManager.h> // Inclui a biblioteca WiFiManager

// --- Configurações do Projeto ---
// NOVO: Define o pino onde o LED verde está conectado
const int ledPin = 13; // D13 na maioria das placas ESP32

// --- Configurações de Rede ---
WiFiUDP udp;
const char* udpAddress = "255.255.255.255";  // broadcast
const int udpPort = 12345;

// --- Controle de Tempo ---
unsigned long lastSend = 0;
const int sendIntervalMs = 100;  // 10 Hz

void setup() {
  Serial.begin(115200);

  // NOVO: Configura o pino do LED como saída
  pinMode(ledPin, OUTPUT);
  // NOVO: Garante que o LED comece desligado
  digitalWrite(ledPin, LOW);

  // Instancia o WiFiManager
  WiFiManager wm;
  //wm.resetSettings();

  // Tenta conectar ao WiFi. Se não conseguir, inicia o portal de configuração.
  if (!wm.autoConnect("PortalDeConfig_ESP32")) {
    Serial.println("Falha ao conectar e o tempo limite expirou. Reiniciando...");
    delay(3000);
    ESP.restart();
  }

  // Se chegou até aqui, o ESP32 está conectado!
  Serial.println("");
  Serial.println("Conectado à rede Wi-Fi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // NOVO: Acende o LED verde para indicar que a conexão foi bem-sucedida
  digitalWrite(ledPin, HIGH);

  udp.begin(udpPort);
}

void loop() {
  // O seu código original para enviar pacotes UDP permanece o mesmo
  unsigned long now = millis();
  if (now - lastSend >= sendIntervalMs) {
    lastSend = now;

    String msg = "ESP32 " + String(now);
    udp.beginPacket(udpAddress, udpPort);
    udp.write((const uint8_t *)msg.c_str(), msg.length());
    udp.endPacket();

    Serial.println("Enviado: " + msg);
  }
}
