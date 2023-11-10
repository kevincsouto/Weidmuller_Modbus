
#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial modbusSerial(8,9);

// Variaveis uteis para o recebimento de mensagem
unsigned long tempo = 0; // "Timer" para saber se a msg acabou
String buf = ""; // String que recebe a mensagem
bool recebeu = false; // Booleana para saber se a msg foi iniciada

// Pino RE e DE do módulo RS485 (desnecessário no simulador)
#define RE_DE 2

// Valor inteiro a ser salvo no registrador 
uint8_t valor = 0xAA55;

unsigned int CRC16(uint8_t msg[], uint8_t len);

void setup()
{
  pinMode(RE_DE, OUTPUT); // pino 8 configurado como saída
  digitalWrite(RE_DE, LOW); // Coloca os pinos RE_DE em LOW para receber os dados

  // Inicia a comunicação serial
  Serial.begin(9600);
  modbusSerial.begin(38400);
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

  // Se está a 100ms sem receber nada e a mensagem foi iniciada
  if((millis()-tempo > 100) && recebeu)
  {
    //--------------------------- RECEBIMENTO ---------------------------------------------
    // Escreve os dados da mensagem em hexadecimal
    for(int i = 0; i < buf.length(); i++)
    {
      Serial.print(" ");
      Serial.print(uint8_t(buf[i]), HEX);
      
      if(i == 7)
      { 
        // Pula a linha quando i = 10
        Serial.println();
      }
    }


    // Analisa a solicitação Modbus recebida
    if (buf.length() == 8 && buf[1] == 0x03) {
      // Verifica se a mensagem é uma solicitação de leitura de registrador
      uint16_t registrador = (uint16_t(buf[2]) << 8) + uint16_t(buf[3]);

      if (registrador == 0x006B) {
        // A mensagem é válida; prepare a resposta
        uint8_t resposta[5] = {0x01, 0x03, 0x02, 0x00, 0x00}; // Endereço SLAVE, código da função e tamanho do campo de dados
        
        resposta[4] = 0xAA; // Dado a ser enviado como resposta
        
        // Calcula o CRC com base na mensagem de resposta
        unsigned int crc = CRC16(resposta, 5);

        digitalWrite(RE_DE, HIGH); // Coloca os pinos em HIGH para enviar os dados
        
        // Imprima os dados da resposta
        for (int i = 0; i < 5; i++) {
          Serial.print(resposta[i], HEX);
          Serial.print(" ");
        }
        Serial.print(crc, HEX);
        Serial.println();

        // Envie a resposta
        for (int i = 0; i < 5; i++) {
          modbusSerial.write(resposta[i]);
        }

        // Envie o CRC
        modbusSerial.write(crc & 0xFF); // Byte menos significativo do CRC
        modbusSerial.write((crc >> 8) & 0xFF); // Byte mais significativo do CRC
        modbusSerial.flush(); // Aguarde os dados serem enviados

        digitalWrite(RE_DE, LOW); // Coloca os pinos RE_DE em LOW novamente para receber os dados
      }


    }
    // Reseta os parâmetros
    buf = "";
    recebeu = false;
  }
}


// Calcula o CRC: Recebe a mensagem em formato de vetor e o tamanho da mensagem (quantos bytes)
unsigned int CRC16(uint8_t msg[], uint8_t len)
{  
  unsigned int crc = 0xFFFF;
  for (uint8_t pos = 0; pos < len; pos++)  {
    crc ^= msg[pos];    // Faz uma XOR entre o LSByte do CRC com o byte de dados atual
 
    for (uint8_t i = 8; i != 0; i--) {    // Itera sobre cada bit
      if ((crc & 0b1) != 0) {      // Se o LSB for 1:
        crc >>= 1;                  // Desloca para a direita
        crc ^= 0xA001;              // E faz XOR com o polinômio 0xA001 (1010 0000 0000 0001 ): x16 + x15 + x2 + 1
      }else{                      // Senão:
        crc >>= 1;                  // Desloca para a direita
      }
    }
  }

  // O formato retornado já sai invertido (LSByte primeiro que o MSByte)
  return crc;
}