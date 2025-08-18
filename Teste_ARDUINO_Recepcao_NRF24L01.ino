//Importação das bibliotecas utilizadas
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // Definição dos pinos CE e CSN
const byte address[6] = "00001"; //Define o endereço de comunicação

void setup() {
  Serial.begin(9600); //Inicia o Monitor serial com uma taxa de 9600
  radio.begin(); //Inicializa o módulo NRF24L01 e configura a comunicação SPI para começar a operar.
  radio.openReadingPipe(0, address); //Abre o canal de recepção no endereço especificado
  radio.setPALevel(RF24_PA_MIN); //Define o nível de potência de transmissão como mínimo (menor alcance e consumo)
  radio.startListening(); //Ativa o modo de recepção

  pinMode(7, OUTPUT); //Define o LED verde como saída
  pinMode(6, OUTPUT); //Define o LED vermelho como saída
}

void loop() {
  digitalWrite(7, LOW); //Apaga o LED verde ao entrar no loop
  digitalWrite(6, LOW); //Apaga o LED vermelho ao entrar no loop

  if (radio.available()) {
    char text[32] = ""; //Cria a variável texto, para armazenar a mensagem recebida
    radio.read(&text, sizeof(text)); //Recebe a mensagem e armazena na variável "text"

    if (strcmp(text, "Mensagem ESP32") == 0){ //Caso a mensagem recebida seja a correta
      Serial.print("Recebido: ");//Mostra o texto no serial monitor
      Serial.println(text);
      digitalWrite(7, HIGH);//Acende o LED verde
      delay(5000);//Delay de 5 segundos
    } 
    else{//Caso a mensagem recebida seja a incorreta
      digitalWrite(6, HIGH);//Acende o LED vermelho
      Serial.println("Mensagem Incorreta!");//Emissão de mensagem incorreta recebida
      delay(5000);//Delay de 5 segundos
    }
    digitalWrite(7, LOW); //Apaga o LED verde
    digitalWrite(6, LOW); //Apaga o LED vermelho
    delay(10000); //Delay de 10 segundos
    
  }
}