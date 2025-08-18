#include "RF24.h" //Inclui a biblioteca para controlar o módulo NRF24L01
#include <SPI.h> //Inclui a bilioteca SPI, necessária para entre o esp32 e o NRF24L01 

// Definições para o primeiro módulo
RF24 radio(4, 5, 16000000);   //Define os pinos CE, CSN e a velocidade de comunicação SPI (16MHz)
byte hopping_channel[40];  // Array para armazenar a primeira metade de canais

// Definições para o segundo módulo
RF24 radio2(21, 22, 16000000);  //Define os pinos CE, CSN e a velocidade de comunicação SPI (16MHz)
byte hopping_channel2[39];  // Array para armazenar a segunda metade de canais

byte ptr_hop = 0;  // Índice para controlar o avanço pelo primeiro intervalo do espectro
byte ptr_hop2 = 0; // Índice para controlar o avanço pelo segundo intervalo do espectro

void setup(void) {
  Serial.begin(115200); //Inicia a comunicação serial em 115200 bps
  SPI.begin(); //Iniciar a comunicação SPI no barramento VSPI

  // Preenche os arrays com todos os canais Bluetooth (0 a 78)
  for (int i = 0; i < 40; i++) {
    hopping_channel[i] = i;  // Canais 0 a 39
  }
  for (int i = 0; i < 39; i++) {
    hopping_channel2[i] = i + 40;  // Canais 40 a 78
  }

  RF();//Invoca a função RF
}

void RF() {
  pinMode(2, OUTPUT); //Configura o pino 2 como saída (LED ded verificação)
  digitalWrite(2, HIGH); //Ativa o led ao iniciar o envio de pacotes
  radio.begin(); //Inicializa o primeiro rádio
  radio.startConstCarrier(RF24_PA_MAX, hopping_channel[0]); //Ativa o portador contínuo na potência máxima e canal 0
  radio.setDataRate(RF24_2MBPS); //Define a taxa de dados como 2Mpbs

  radio2.begin(); //Inicializa o segundo rádio
  radio2.startConstCarrier(RF24_PA_MAX, hopping_channel2[0]); //Ativa o portador contínuo na potência máxima e canal 45
  radio2.setDataRate(RF24_2MBPS); //Define a taxa de dados como 2Mpbs
}

void loop(void) {
  ptr_hop++; //Incrementa o índice do canal para o intervalo 1
  if (ptr_hop >= sizeof(hopping_channel)) ptr_hop = 0;  // Se o índice chegar ao tamanho do intervalo, retoma seu valor para 0
  radio.setChannel(hopping_channel[ptr_hop]);           // Alterna o canal do primeira rádio


  ptr_hop2++; //Incrementa o índice do canal para o intervalo 2
  if (ptr_hop2 >= sizeof(hopping_channel2)) ptr_hop2 = 0; // Se o índice chegar ao tamanho do intervalo, retoma seu valor para 0
  radio2.setChannel(hopping_channel2[ptr_hop2]);        // Alterna o canal do segundo rádio

}