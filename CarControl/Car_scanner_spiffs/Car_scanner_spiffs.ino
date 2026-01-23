#include <SPI.h>
#include <mcp_can.h>
#include "SPIFFS.h"

// --- CONFIGURAÇÃO HARDWARE ---
const int SPI_CS_PIN = 14; // Seu pino CS do MCP2515
MCP_CAN CAN(SPI_CS_PIN);

// --- CONFIGURAÇÃO DO BUFFER ---
// Acumula dados na RAM antes de salvar na Flash para ganhar velocidade
String dataBuffer = "";
const int BUFFER_THRESHOLD = 2048; // Salva a cada ~2KB de dados acumulados
const char* LOG_FILE = "/log_can.txt";

// --- VELOCIDADES ---
const uint8_t CAN_SPEEDS[] = {CAN_500KBPS, CAN_250KBPS, CAN_125KBPS};
const char* CAN_SPEED_NAMES[] = {"500kbps", "250kbps", "125kbps"};

void setup() {
  Serial.begin(115200); // Velocidade máxima para descarregar rápido depois
  
  // 1. INICIA SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("❌ Falha ao montar SPIFFS");
    return;
  }
  Serial.println("✅ SPIFFS montado.");
  Serial.println("Envie 'L' para ler o log salvo.");
  Serial.println("Envie 'D' para deletar o log e começar do zero.");

  // 2. INICIA CAN
  bool canInit = false;
  for (size_t i = 0; i < 3; i++) {
    if (CAN.begin(MCP_ANY, CAN_SPEEDS[i], MCP_8MHZ) == CAN_OK) {
      Serial.print("✅ CAN Conectado: ");
      Serial.println(CAN_SPEED_NAMES[i]);
      canInit = true;
      break;
    }
  }

  if (!canInit) {
    Serial.println("❌ Erro no MCP2515. Verifique cabos.");
    // Não trava o código, pois você pode querer ler o log mesmo sem CAN
  }

  CAN.init_Mask(0, 0, 0x00000000); // Filtro aberto (Pega tudo)
  CAN.init_Mask(1, 0, 0x00000000);
  for (int i = 0; i < 6; i++) CAN.init_Filt(i, 0, 0x00000000);
  CAN.setMode(MCP_NORMAL);

  Serial.println("--- GRAVANDO DADOS NO SPIFFS ---");
}

void loop() {
  // --- COMANDOS VIA SERIAL (Para usar em casa) ---
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'L' || cmd == 'l') dumpLog();
    if (cmd == 'D' || cmd == 'd') deleteLog();
  }

  // --- LEITURA DO CAN ---
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    long unsigned int rxId;
    unsigned char len = 0;
    unsigned char rxBuf[8];

    CAN.readMsgBuf(&rxId, &len, rxBuf);

    // Formata linha CSV: Time;ID;DLC;D0...D7
    String line = String(millis()) + ";";
    
    // Tratamento ID Estendido
    if ((rxId & 0x80000000) == 0x80000000) rxId = (rxId & 0x1FFFFFFF);
    
    char hexBuffer[10];
    sprintf(hexBuffer, "%03X", rxId);
    line += String(hexBuffer) + ";";
    line += String(len) + ";";

    for (byte i = 0; i < len; i++) {
      char byteBuf[4];
      sprintf(byteBuf, "%02X", rxBuf[i]);
      line += String(byteBuf);
      if (i < len - 1) line += " ";
    }
    line += "\n"; // Quebra de linha

    // Adiciona ao Buffer da RAM
    dataBuffer += line;

    // Se o buffer encheu, descarrega no SPIFFS
    if (dataBuffer.length() >= BUFFER_THRESHOLD) {
      flushBufferToSpiffs();
    }
  }
}

// --- FUNÇÃO PARA SALVAR NA FLASH ---
void flushBufferToSpiffs() {
  File file = SPIFFS.open(LOG_FILE, FILE_APPEND); // Abre modo APPEND (adicionar ao final)
  if (!file) {
    Serial.println("Erro ao abrir arquivo para gravar!");
    return;
  }
  
  if (file.print(dataBuffer)) {
    Serial.print("."); // Imprime um ponto para mostrar que salvou um bloco
  } else {
    Serial.println("Falha na escrita!");
  }
  
  file.close();
  dataBuffer = ""; // Limpa a RAM para recomeçar
}

// --- FUNÇÃO PARA LER O ARQUIVO (DUMP) ---
void dumpLog() {
  // Salva qualquer resto que esteja no buffer antes de ler
  if (dataBuffer.length() > 0) flushBufferToSpiffs();

  File file = SPIFFS.open(LOG_FILE, FILE_READ);
  if (!file) {
    Serial.println("Nenhum log encontrado.");
    return;
  }

  Serial.println("\n--- INICIO DO ARQUIVO ---");
  while (file.available()) {
    Serial.write(file.read());
  }
  Serial.println("\n--- FIM DO ARQUIVO ---");
  file.close();
}

// --- FUNÇÃO PARA LIMPAR ---
void deleteLog() {
  if (SPIFFS.remove(LOG_FILE)) {
    Serial.println("Log deletado com sucesso!");
    dataBuffer = "";
  } else {
    Serial.println("Falha ao deletar log.");
  }
}
