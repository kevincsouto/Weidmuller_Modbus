// Importa a bibilioteca do display
//#include <LiquidCrystal.h>
// Cria uma instancia do display com os pinos
//LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial modbusSerial(8,9);

// Variaveis uteis para o programa
unsigned long tempo = 0; // "Timer" para saber se a msg acabou
String buf = ""; // String que recebe a mensagem
bool recebeu = false; // Booleana para saber se a msg foi iniciada

// Pino RE e DE do módulo RS485 (desnecessário no simulador)
#define RE_DE 2

void setup()
{
  pinMode(RE_DE, OUTPUT); // pino 8 configurado como saída
  digitalWrite(RE_DE, LOW); // Coloca os pinos RE_DE em LOW para receber os dados

  // Inicia a comunicação serial
  Serial.begin(9600);
  modbusSerial.begin(38500);

  // Inicializa o LCD:
  //lcd.begin(16, 2);
  //lcd.clear();
}


void loop()
{
  // Se algum dado foi recebido
  if(modbusSerial.available()>0){
    // Le o dado e adiciona à String 'buf'
    char recebido = modbusSerial.read();
    buf += recebido;

    // Reseta  o "timer" e indica que a mensagem foi iniciada
    tempo = millis();
    recebeu = true;
  }

  // Se está a 4ms sem receber nada e a mensagem foi iniciada
  if((millis()-tempo > 4) && recebeu){
   
    // Escreve os dados da mensagem em hexadecimal
    //lcd.setCursor(0,0);
    for(unsigned int i = 0; i<buf.length();i++){
      /*
      if(i==6){ // Pula a linha quando i = 6
        lcd.setCursor(0,1);
      }
      */

      Serial.print(uint8_t(buf[i]), HEX);
     
      if(i != 2 && i != buf.length()-2){ // Só cria um espaço se não for o endereço do registrador ou o CRC
        Serial.print(" ");
      }

    }
    Serial.println();
   
    //// OS DADOS ESTÃO ARMAZENADOS EM:
    // Endereço do escravo: em buf[0]
    // Código da função: buf[1]
    // Endereço do registrador: buf[2] e buf[3]
    //  Para juntar os dois, podemos fazer o seguinte:
    int endereco = (uint8_t(buf[2]) << 8) | uint8_t(buf[3]);
    // Os dados da mensagem vem na sequência (buf[4]...)
    // O CRC está localizado nos ultimos dois bytes: buf[buf.length()-2] e buf[buf.length()-1]


    // Reseta os parâmetros
    buf = ""; // Limpa a mensagem
    recebeu = false; // Reseta a booleana
  }
}