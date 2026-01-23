#include <SPI.h>
#include <mcp_can.h>
#include <FS.h>
#include <LittleFS.h>

const int SPI_CS_PIN = 14; 
MCP_CAN CAN(SPI_CS_PIN);

String dataBuffer = "";
const int BUFFER_THRESHOLD = 2048; 
const char* LOG_FILE = "/log_can.txt";

// --- VELOCIDADES ---
const uint8_t CAN_SPEEDS[] = {CAN_500KBPS, CAN_250KBPS, CAN_125KBPS};
const char* CAN_SPEED_NAMES[] = {"500kbps", "250kbps", "125kbps"};

// ============================================================
// FUNÇÕES AUXILIARES (Definidas ANTES para evitar erro)
// ============================================================

void flushBufferToSpiffs() {
  File file = LittleFS.open(LOG_FILE, FILE_APPEND); 
  if (!file) {
    Serial.println("Erro ao abrir arquivo para gravar!");
    return;
  }
  
  if (file.print(dataBuffer)) {
    Serial.print("."); 
  } else {
    Serial.println("Falha na escrita!");
  }
  
  file.close();
  dataBuffer = ""; // Limpa o buffer
}

void dumpLog() {
  // Salva o que sobrou no buffer antes de ler
  if (dataBuffer.length() > 0) flushBufferToSpiffs();

  File file = LittleFS.open(LOG_FILE, FILE_READ);
  if (!file) {
    Serial.println("Log vazio ou inexistente.");
    return;
  }

  Serial.println("\n--- INICIO DO ARQUIVO ---");
  
  // Buffer de leitura para acelerar o download
  uint8_t buf[64]; 
  while (file.available()) {
    int len = file.read(buf, sizeof(buf));
    Serial.write(buf, len);
    
    // CORREÇÃO CRÍTICA: Evita que o ESP32 reinicie (Watchdog)
    yield(); 
  }
  
  Serial.println("\n--- FIM DO ARQUIVO ---");
  file.close();
}

void deleteLog() {
  LittleFS.remove(LOG_FILE);
  Serial.println("Log deletado com sucesso.");
  dataBuffer = "";
}

// ============================================================
// SETUP E LOOP
// ============================================================

void setup() {
  Serial.begin(115200);
  
  // Inicia LittleFS com formatação automática se falhar
  if (!LittleFS.begin(true)) {
    Serial.println("Falha grave ao montar LittleFS");
    return;
  }
  Serial.println(" LittleFS montado.");
  Serial.print("Espaço Total: "); Serial.print(LittleFS.totalBytes());
  Serial.print(" | Usado: "); Serial.println(LittleFS.usedBytes());
  
  Serial.println("Envie 'L' para ler | Envie 'D' para deletar");

  // INICIA CAN
  bool canInit = false;
  for (size_t i = 0; i < 3; i++) {
    if (CAN.begin(MCP_ANY, CAN_SPEEDS[i], MCP_8MHZ) == CAN_OK) {
      Serial.print("CAN Conectado: ");
      Serial.println(CAN_SPEED_NAMES[i]);
      canInit = true;
      break;
    }
  }

  if (!canInit) Serial.println("Erro no MCP2515. Modo Offline (só leitura de logs).");

  CAN.init_Mask(0, 0, 0x00000000);
  CAN.init_Mask(1, 0, 0x00000000);
  for (int i = 0; i < 6; i++) CAN.init_Filt(i, 0, 0x00000000);
  CAN.setMode(MCP_NORMAL);

  Serial.println("--- GRAVANDO ---");
}

void loop() {
  // --- COMANDOS VIA SERIAL ---
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

    // Formatação
    if ((rxId & 0x80000000) == 0x80000000) rxId = (rxId & 0x1FFFFFFF);
    
    char tmp[64]; // Usar buffer char é mais seguro que String para memória
    int n = sprintf(tmp, "%lu;%03lX;%d;", millis(), rxId, len);
    
    // Adiciona ao buffer principal
    dataBuffer += String(tmp);

    for (byte i = 0; i < len; i++) {
      sprintf(tmp, "%02X", rxBuf[i]);
      dataBuffer += String(tmp);
      if (i < len - 1) dataBuffer += " ";
    }
    dataBuffer += "\n";

    // Se o buffer encheu, descarrega
    if (dataBuffer.length() >= BUFFER_THRESHOLD) {
      flushBufferToSpiffs();
    }
  }
}
