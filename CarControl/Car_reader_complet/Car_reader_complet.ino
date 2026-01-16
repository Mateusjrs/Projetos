
#include <SPI.h>
#include <mcp_can.h>
#include "can_map.h"

// Definição dos pinos SPI para o ESP32
const int SPI_CS_PIN = 5; 

// Inicializa o objeto MCP_CAN
MCP_CAN CAN(SPI_CS_PIN);

int velocidade = 0;      // km/h
int rpm = 800;           // marcha lenta
int tempMotor = 85;      // °C
int combustivel = 70;    // %
bool portaAberta = false;
bool farolLigado = false;

void updateCarState() {
  // aceleração simples
  if (velocidade < 120) {
    velocidade++;
    rpm += 50;
  }

  if (rpm > 4000) rpm = 4000;
  if (velocidade > 120) velocidade = 120;
}

unsigned long lastSend = 0;

void sendPeriodicFrames() {
  if (millis() - lastSend > 100) { // 100 ms
    sendSpeed();
    sendRPM();
    lastSend = millis();
  }
}
void sendSpeed() {
  byte msg[8] = {velocidade, 0,0,0,0,0,0,0};
  CAN.sendMsgBuf(0x0B4, 0, 8, msg);
}

void sendRPM() {
  int val = rpm * 4;
  byte msg[8] = {
    (byte)(val >> 8),
    (byte)(val & 0xFF),
    0,0,0,0,0,0
  };
  CAN.sendMsgBuf(0x0C0, 0, 8, msg);
}


void handleOBD(unsigned long id, byte *buf) {
  if (id == 0x7DF && buf[1] == 0x01) {
    byte pid = buf[2];

    // Velocidade
    if (pid == 0x0D) {
      byte resp[8] = {0x03, 0x41, 0x0D, velocidade,0,0,0,0};
      CAN.sendMsgBuf(0x7E8, 0, 8, resp);
    }

    // RPM
    if (pid == 0x0C) {
      int val = rpm * 4;
      byte resp[8] = {
        0x04, 0x41, 0x0C,
        (byte)(val >> 8),
        (byte)(val & 0xFF),
        0,0,0
      };
      CAN.sendMsgBuf(0x7E8, 0, 8, resp);
    }
  }
}

void handleSerial() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');

  if (cmd.startsWith("SET SPEED")) {
    velocidade = cmd.substring(10).toInt();
  }
  if (cmd.startsWith("SET RPM")) {
    rpm = cmd.substring(8).toInt();
  }
}
