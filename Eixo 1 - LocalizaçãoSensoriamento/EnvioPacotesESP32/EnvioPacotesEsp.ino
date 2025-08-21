#include <WiFi.h>
#include <WiFiUdp.h>

// --- Configurações do Projeto ---
const int ledPin = 13; 
String macAddress; // Variável global para armazenar o endereço MAC

// --- Configurações da Rede Wi-Fi que o ESP32 vai CRIAR ---
const char* ssid = "ESP32_CSI_Transmitter"; 
const char* password = "password123";      
const int channel = 8;                     

// --- Configurações do Transmissor UDP ---
WiFiUDP udp;
const int udpPort = 4444;                  
const char* broadcastAddress = "255.255.255.255"; 

// --- Controle de Tempo (10 pacotes por segundo) ---
const int sendIntervalMs = 100; 
unsigned long lastSendTime = 0;
unsigned long packetCounter = 0;

void setup() {
  Serial.begin(115200);
  Serial.println();

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); 

  Serial.println("Configurando o ESP32 como Ponto de Acesso (AP)...");

  if (WiFi.softAP(ssid, password, channel)) {
    // CORREÇÃO: Obtemos o MAC aqui, DEPOIS que a rede foi criada com sucesso.
    macAddress = WiFi.softAPmacAddress();

    Serial.println("----------------------------------------------------");
    Serial.print("Rede Wi-Fi '");
    Serial.print(ssid);
    Serial.println("' criada com sucesso!");
    Serial.print("Endereço MAC deste ESP32: "); // Apenas para confirmar no início
    Serial.println(macAddress);
    Serial.print("Canal de Operação: ");
    Serial.println(WiFi.channel());
    Serial.print("Endereço IP do ESP32: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("----------------------------------------------------");
    Serial.println("Iniciando envio de pacotes UDP...");

    digitalWrite(ledPin, HIGH);

  } else {
    Serial.println("Falha ao criar o Ponto de Acesso!");
    while(1); 
  }

  udp.begin(udpPort);
}

void loop() {
  unsigned long currentTime = millis();

  if (currentTime - lastSendTime >= sendIntervalMs) {
    lastSendTime = currentTime;

    String message = "Packet #" + String(packetCounter);
    
    udp.beginPacket(broadcastAddress, udpPort);
    udp.print(message);
    udp.endPacket();

    // Agora esta linha usará o endereço MAC correto que foi salvo no setup
    Serial.println("Enviado por [" + macAddress + "]: " + message);
    
    packetCounter++;
  }
}
