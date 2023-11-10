
#include <Arduino.h>

#include <SoftwareSerial.h>

SoftwareSerial modbusSerial(8,9);

// Variaveis uteis para o recebimento de mensagem
unsigned long tempo = 0; // "Timer" para saber se a msg acabou
String buf = ""; // String que recebe a mensagem
bool recebeu = false; // Booleana para saber se a msg foi iniciada

// Pino RE e DE do módulo RS485 (desnecessário no simulador)
#define RE_DE 2

// Vetor booleano com o estado das lampadas
bool lampadas[8] = {1,0,1,0,1,0,1,0};

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

  // Se está a 4ms sem receber nada e a mensagem foi iniciada
  if((millis()-tempo > 4) && recebeu){
    //--------------------------- RECEBIMENTO ---------------------------------------------
    // Escreve os dados da mensagem em hexadecimal
    //lcd.setCursor(0,0);
    for(unsigned int i = 0; i<buf.length();i++){
      if(i==6){ // Pula a linha quando i = 6
        //lcd.setCursor(0,1);
        Serial.println();
      }
      Serial.print(uint8_t(buf[i]), HEX);
     
      if(i != 2 && i != buf.length()-2){ // Só cria um espaço se não for o endereço do registrador ou o CRC
        Serial.print(" ");
      }
    }

    // --------------------------- ENVIO ---------------------------
    // Cria um vetor para guardar a mensagem de envio (o tamanho de 15 é arbitrário)
    uint8_t msg[15];
    // Cria uma variável para guardar o tamanho do vetor
    uint8_t tamanho = 3;
    msg[0] = uint8_t(buf[0]); // Endereço SLAVE
    msg[1] = uint8_t(buf[1]); // Codigo da função
    int registrador = (uint8_t(buf[2]) << 8) + uint8_t(buf[3]); // Endereço do registrador
   
    if(msg[1] == 1){ // Se o codigo for de leitura de saída digital

      // Calcula quantas saídas o master quer ler
      int n = (uint8_t(buf[4]) << 8) + uint8_t(buf[5]);
      // Coloca na mensagem quantos bytes de dados serão enviados (quantidade de saidas/8, já que cada saida é um bit e serão 8 saidas por byte)
      msg[2] = int(n/8);

      // Variavel para saber em qual posição do vetor da mensagem gravar os valores das lampadas
      int k = registrador;
      // Forma os bytes de estado das lampadas
      for(uint8_t i = 0; i<msg[2];i++){
        uint8_t dado=0;
        // Forma o byte atual
        for(uint8_t j=0;j<8;j++){
          dado += lampadas[k] << j;
          k++;
        }
        // Armazena o byte de dados na mensagem e soma 1 ao tamanho
        msg[tamanho++] = dado;
      }
      // Por fim, calcula o CRC com base na mensagem e no tamanho dela
      unsigned int crc = CRC16(msg, tamanho);

      digitalWrite(RE_DE, HIGH); // Coloca os pinos em HIGH para enviar os dados

      modbusSerial.write(msg[0]); // Endereço SLAVE
      modbusSerial.write(msg[1]); // Codigo da função
      modbusSerial.write(msg[2]); // Quantidade de bytes de dados
      for(uint8_t i = 3; i< tamanho; i++){
        modbusSerial.write(msg[i]); // Estado das lampadas
      }
      modbusSerial.write(crc&0b11111111); // Envia os 8 LSbits do CRC primeiro
      modbusSerial.write(crc>>8); // Envia os MSbits do CRC depois (são trocados mesmo)
      modbusSerial.flush(); // Aguarda os dados serem enviados

      digitalWrite(RE_DE, LOW); // Coloca os pinos RE_DE em LOW novamente para receber os dados
    }


    // Reseta os parâmetros
    buf = "";
    recebeu = false;
  }
}


