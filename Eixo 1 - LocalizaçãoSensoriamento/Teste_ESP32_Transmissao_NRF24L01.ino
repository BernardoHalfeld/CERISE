//Importação das bibliotecas utilizadas
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Definição dos pinos do NRF24L01 para ESP32
#define CE_PIN 4 
#define CSN_PIN 5 
#define SCK 18 
#define MISO 19  
#define MOSI 23 

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001"; //Define o endereço de comunicação
bool alternar = false; //Define a variável de controle

void setup() {
  Serial.begin(115200); //Inicia o Monitor serial com uma taxa de 115200
  SPI.begin(SCK, MISO, MOSI, CSN_PIN);  // Inicializa SPI no ESP32
  radio.begin(); //Inicializa o módulo NRF24L01 e configura a comunicação SPI para começar a operar.
  radio.setPALevel(RF24_PA_MIN); //Define o nível de potência de transmissão como mínimo (menor alcance e consumo)
  radio.openWritingPipe(address); //Define o endereço do canal de transmissão
  radio.stopListening(); //Desativa o modo de recepção

  pinMode(2, OUTPUT); //Define o LED como saída
}

void loop() {
  digitalWrite(2, LOW); //Apaga o LED ao entrar no loop
  const char* text; //Define a variável texto
  
  //Laço condicional, sempre que alterar for true, envia a mensagem correta, quando
  //é false, envia a mensagem incorreta
  if (alternar) {
    text = "Mensagem ESP32"; // Mensagem correta
  } else {
    text = "Mensagem BlaBla"; // Mensagem errada
  }
  bool sucesso = radio.write(text, strlen(text) + 1); // Envia mensagem

  if (sucesso) { //Caso a mensagem tenha sido enviada corretamente
    digitalWrite(2, HIGH); //Acende-se o led verde
    Serial.println("Mensagem enviada com sucesso!");//Emissão de mensagem de sucesso
    delay(5000);//Delay de 5 segundos
  } 
  else { //Caso a mensagem não tenha sido enviada corretamente
    Serial.println("Falha ao enviar mensagem.");//Emissão de mensagem de erro
    delay(5000);//Delay de 5 segundos
  }
  alternar = !alternar;//Muda o valor da variável para alteranar a mensagem a ser enviada
  digitalWrite(2, LOW);//Apaga o led
  delay(10000);//Delay de 10 segundos
}

