# Controle do Carregador Wallbox Weidmüller via RS485 Modbus

## Descrição

Este repositório contém o código-fonte para controlar o carregador Wallbox da Weidmüller usando a comunicação serial RS485 com o protocolo Modbus. O código foi desenvolvido para ser executado em uma plataforma Arduino.

## Pré-requisitos

- Placa Arduino compatível
- Módulo RS485 para comunicação serial
- Biblioteca Modbus (insira o link para a biblioteca que você está usando)

## Instalação

1. **Clone este repositório:** `git clone https://github.com/seu-usuario/seu-repositorio.git`
2. **Abra o arquivo `wallbox_control.ino` no Arduino IDE.**
3. **Conecte sua placa Arduino ao computador.**
4. **Selecione o tipo de placa e a porta serial no Arduino IDE.**
5. **Carregue o código no Arduino.**

## Configuração

Certifique-se de configurar adequadamente as variáveis no início do arquivo `wallbox_control.ino` para atender à sua configuração específica.

```cpp
// Configurações RS485
#define RE_DE 2
SoftwareSerial modbusSerial(8, 9); // Pinos RX, TX

// Outras configurações
// ...

