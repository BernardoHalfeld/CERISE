/*
 * Código para usar um ESP32 com três módulos NRF24L01
 * para varrer os canais de 0 a 78 com LEDs de status.
 *
 * Estratégia: Cada módulo varre um terço do espectro de forma independente,
 * dividindo a carga para cobrir a faixa de canais.
 *
 * Rádio 1 (Pinos 4, 5)   -> LED 1 (Pino 13) -> Canais 0-25
 * Rádio 2 (Pinos 26, 27) -> LED 2 (Pino 12) -> Canais 26-51
 * Rádio 3 (Pinos 32, 33) -> LED 3 (Pino 14) -> Canais 52-78
 *
 */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// --- PINAGEM DOS RÁDIOS ---
const int pinoCE1 = 4;
const int pinoCSN1 = 5;
const int pinoCE2 = 26;
const int pinoCSN2 = 27;
const int pinoCE3 = 32;
const int pinoCSN3 = 33;

// --- PINAGEM DOS LEDs ---
const int ledPin1 = 13;
const int ledPin2 = 12;
const int ledPin3 = 14;

// --- INSTÂNCIAS DOS RÁDIOS ---
RF24 radio1(pinoCE1, pinoCSN1);
RF24 radio2(pinoCE2, pinoCSN2);
RF24 radio3(pinoCE3, pinoCSN3);

// --- CONTADORES DE CANAIS ---
byte canalAtual1 = 0;
byte canalAtual2 = 26;
byte canalAtual3 = 52;

// Função para configurar um rádio e seu LED de status
void configurarRadioScanner(RF24 &radio, int ledPin, const char* nomeRadio, byte canalInicial) {
  Serial.print("Configurando ");
  Serial.println(nomeRadio);

  if (radio.begin()) {
    radio.setDataRate(RF24_2MBPS);
    radio.setPALevel(RF24_PA_MAX);
    radio.setAutoAck(false);
    radio.setRetries(0, 0);
    radio.startConstCarrier(RF24_PA_MAX, canalInicial);
    
    digitalWrite(ledPin, HIGH); // LIGA o LED para indicar sucesso
    Serial.print(nomeRadio);
    Serial.println(" configurado e transmitindo. LED LIGADO.");
  } else {
    digitalWrite(ledPin, LOW); // Mantém o LED DESLIGADO para indicar falha
    Serial.print("Falha ao iniciar o ");
    Serial.println(nomeRadio);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("--- Scanner de Espectro Setorizado (0-78) com 3x NRF24L01 ---");

  // Configura os pinos dos LEDs como saída e os desliga inicialmente
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, LOW);
  digitalWrite(ledPin3, LOW);

  // Configura cada rádio
  configurarRadioScanner(radio1, ledPin1, "Radio 1", canalAtual1);
  configurarRadioScanner(radio2, ledPin2, "Radio 2", canalAtual2);
  configurarRadioScanner(radio3, ledPin3, "Radio 3", canalAtual3);

  Serial.println("----------------------------------------");
  Serial.println("Varredura de canais iniciada.");
}

void loop() {
  // --- VARREDURA DO MÓDULO 1 (Canais 0-25) ---
  canalAtual1++;
  if (canalAtual1 > 25) {
    canalAtual1 = 0;
  }
  radio1.setChannel(canalAtual1);

  // --- VARREDURA DO MÓDULO 2 (Canais 26-51) ---
  canalAtual2++;
  if (canalAtual2 > 51) {
    canalAtual2 = 26;
  }
  radio2.setChannel(canalAtual2);

  // --- VARREDURA DO MÓDULO 3 (Canais 52-78) ---
  canalAtual3++;
  if (canalAtual3 > 78) {
    canalAtual3 = 52;
  }
  radio3.setChannel(canalAtual3);
}
