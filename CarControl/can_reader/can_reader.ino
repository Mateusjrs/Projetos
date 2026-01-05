// =================================================================
// CÓDIGO PARA O NÓ 2: LEITOR / TRADUTOR (SCANNER) - VERSÃO COMPLETA
// HARDWARE: ESP32 + MCP2515 (ou TJA1050/VP230 como transceptor)
// IDE: Arduino IDE
// =================================================================

#include <SPI.h>
#include <mcp_can.h>
#include "can_map.h"

const uint8_t CAN_SPEEDS[] = {
  CAN_500KBPS,
  CAN_250KBPS,
  CAN_125KBPS
};

const char* CAN_SPEED_NAMES[] = {
  "500 kbps",
  "250 kbps",
  "125 kbps"
};

uint8_t detectedCanSpeed = CAN_500KBPS;
bool canDetected = false;


// Definição dos pinos SPI para o ESP32
const int SPI_CS_PIN = 5;

// Inicializa o objeto MCP_CAN
MCP_CAN CAN(SPI_CS_PIN);

// Array para armazenar o último valor decodificado de cada sinal proprietário
float last_prop_values[NUM_CAN_SIGNALS];
// Array para armazenar o último valor decodificado de cada PID OBD-II
float last_obd_values[NUM_OBD_PIDS];

// Variáveis de controle de tempo
unsigned long last_print_time = 0;
const long print_interval = 500; // Imprime a cada 500ms

unsigned long last_obd_request_time = 0;
const long obd_request_interval = 200; // Solicita um PID a cada 100ms

// Índice do PID OBD-II a ser solicitado na próxima iteração
size_t current_obd_pid_index = 0;


#define ISOTP_BUFFER_SIZE 64

uint8_t isotp_buffer[ISOTP_BUFFER_SIZE];
uint16_t isotp_len = 0;
uint16_t isotp_offset = 0;
bool isotp_active = false;
uint8_t isotp_expected_sn = 1;

#define MAX_SUPPORTED_PIDS 64

bool supported_pids[256];   // índice = PID
bool pid_discovery_done = false;

uint8_t pid_discovery_requests[] = {0x00, 0x20, 0x40};
uint8_t pid_discovery_index = 0;


// ===============================
// OBD ADDRESS FALLBACK
// ===============================
const uint16_t OBD_REQUEST_IDS[] = {
  0x7DF, // broadcast
  0x7E0, // ECU engine
  0x7E1  // ECU alternativa
};

uint8_t obd_addr_index = 0;
unsigned long last_obd_response_time = 0;
bool obd_response_received = false;

#define OBD_RESPONSE_TIMEOUT 300 // ms


void setup() {
  Serial.begin(115200);
  delay(500);

  for (size_t i = 0;
       i < sizeof(CAN_SPEEDS) / sizeof(CAN_SPEEDS[0]);
       i++) {
    if (tryCanSpeed(CAN_SPEEDS[i], CAN_SPEED_NAMES[i])) {
      detectedCanSpeed = CAN_SPEEDS[i];
      canDetected = true;
      break;
    }
  }

  if (!canDetected) {
    Serial.println("❌ Nenhuma velocidade CAN detectada");
    while (1); // trava
  }

  Serial.println("✅ CAN inicializado com sucesso");

  // ⚠️ Depois disso você configura filtros normalmente
  CAN.init_Mask(0, 0, 0x7FF);
  CAN.init_Filt(0, 0, 0x0B6);
  CAN.init_Filt(1, 0, 0x0F6);

  CAN.init_Mask(1, 0, 0x7F8);
  CAN.init_Filt(2, 1, 0x7E8);

  CAN.setMode(MCP_NORMAL);
}

bool tryCanSpeed(uint8_t speed, const char* label) {

  Serial.print("Tentando CAN ");
  Serial.println(label);

  if (CAN.begin(MCP_ANY, speed, MCP_8MHZ) != CAN_OK) {
    Serial.println("  Falha ao inicializar MCP2515");
    return false;
  }

  CAN.setMode(MCP_NORMAL);

  unsigned long start = millis();

  // Espera até 1 segundo por qualquer frame CAN
  while (millis() - start < 1000) {
    if (CAN_MSGAVAIL == CAN.checkReceive()) {
      Serial.print("  CAN detectado em ");
      Serial.println(label);
      return true;
    }
  }

  Serial.println("  Nenhuma mensagem detectada");
  return false;
}


// =================================================================
// LÓGICA DE SOLICITAÇÃO OBD-II
// =================================================================

void sendObdRequest() {
  if (NUM_OBD_PIDS == 0) return;

  // Seleciona o próximo PID a ser solicitado
  const ObdPid* pid_map = &OBD_PIDS[current_obd_pid_index];

  // Formato da Solicitação OBD-II (Mode 01): [DLC] [Num Bytes] [Mode] [PID] [0] [0] [0] [0]
  // O DLC é 8, mas o número de bytes de dados é 2 (Mode + PID)
  uint8_t request_data[8] = {0x02, pid_map->mode, pid_map->pid, 0x00, 0x00, 0x00, 0x00, 0x00};

  // Envia a solicitação para o ID de broadcast (0x7DF)
  byte sndStat = CAN.sendMsgBuf(CAN_ID_OBD_REQUEST, 0, 8, request_data);

  if (sndStat == CAN_OK) {
    Serial.print("OBD Request Enviado: ");
    Serial.print(pid_map->name);
    Serial.print(" (0x");
    Serial.print(pid_map->pid, HEX);
    Serial.println(")");
  } else {
    Serial.print("Erro ao enviar OBD Request: ");
    Serial.println(sndStat);
  }

  // Move para o próximo PID para a próxima solicitação
  current_obd_pid_index = (current_obd_pid_index + 1) % NUM_OBD_PIDS;
}


void sendPidDiscoveryRequest() {

  uint8_t pid = pid_discovery_requests[pid_discovery_index];

  uint8_t req[8] = {
    0x02,       // número de bytes
    0x01,       // Mode 01
    pid,        // PID base (00, 20, 40)
    0, 0, 0, 0, 0
  };

  CAN.sendMsgBuf(CAN_ID_OBD_REQUEST, 0, 8, req);

  Serial.print("PID discovery request: 01 ");
  Serial.println(pid, HEX);
}


void sendIsoTp(long txId, const uint8_t *payload, uint16_t length) {

  uint8_t frame[8] = {0};

  // ===============================
  // SINGLE FRAME
  // ===============================
  if (length <= 7) {
    frame[0] = length;
    memcpy(&frame[1], payload, length);
    CAN.sendMsgBuf(txId, 0, 8, frame);
    return;
  }

  // ===============================
  // FIRST FRAME
  // ===============================
  frame[0] = 0x10 | ((length >> 8) & 0x0F);
  frame[1] = length & 0xFF;

  uint8_t firstLen = 6;
  memcpy(&frame[2], payload, firstLen);
  CAN.sendMsgBuf(txId, 0, 8, frame);

  uint16_t offset = firstLen;
  uint8_t seq = 1;

  delay(5); // tempo mínimo antes de CF (simples)

  // ===============================
  // CONSECUTIVE FRAMES
  // ===============================
  while (offset < length) {
    memset(frame, 0, 8);
    frame[0] = 0x20 | (seq & 0x0F);

    uint8_t chunk = min((uint16_t)7, (uint16_t)(length - offset));
    memcpy(&frame[1], &payload[offset], chunk);

    CAN.sendMsgBuf(txId, 0, 8, frame);

    offset += chunk;
    seq++;

    delay(5); // STmin simples
  }
}


void sendObdWithFallback(const uint8_t *payload, uint8_t length) {

    uint16_t txId = OBD_REQUEST_IDS[obd_addr_index];

    sendIsoTp(txId, payload, length);

    Serial.print("OBD request enviado para ID 0x");
    Serial.println(txId, HEX);
}


void updateObdAddressFallback() {

    if (obd_response_received) return;

    if (millis() - last_obd_response_time > OBD_RESPONSE_TIMEOUT) {

        obd_addr_index++;

        if (obd_addr_index >=
            sizeof(OBD_REQUEST_IDS) / sizeof(OBD_REQUEST_IDS[0])) {

            obd_addr_index = 0; // volta para broadcast
        }

        last_obd_response_time = millis();

        Serial.print("🔁 Fallback para OBD ID 0x");
        Serial.println(OBD_REQUEST_IDS[obd_addr_index], HEX);
    }
}


// =================================================================
// LÓGICA DE PROCESSAMENTO DE MENSAGENS
// =================================================================

bool processIsoTpFrame(long rxId, const uint8_t *data, uint8_t len) {

  uint8_t pciType = data[0] >> 4;

  // ===============================
  // SINGLE FRAME
  // ===============================
  if (pciType == 0x0) {
    uint8_t payloadLen = data[0] & 0x0F;
    memcpy(isotp_buffer, &data[1], payloadLen);
    isotp_len = payloadLen;
    return true;
  }

  // ===============================
  // FIRST FRAME
  // ===============================
  if (pciType == 0x1) {
    isotp_len = ((data[0] & 0x0F) << 8) | data[1];
    isotp_offset = len - 2;

    memcpy(isotp_buffer, &data[2], isotp_offset);

    isotp_expected_sn = 1;
    isotp_active = true;

    // Envia Flow Control (CTS)
    // Envia Flow Control (CTS)
    // 0x30 = Continue To Send
    // 0x00 = Block Size (sem limite)
    // 0x00 = STmin
    uint8_t fc[8] = {0x30, 0x00, 0x00, 0, 0, 0, 0, 0};

    // Para respostas OBD padrão, enviar para ECU Engine (0x7E0)
    CAN.sendMsgBuf(0x7E0, 0, 8, fc);


    return false;
  }

  // ===============================
  // CONSECUTIVE FRAME
  // ===============================
  if (pciType == 0x2 && isotp_active) {
    uint8_t sn = data[0] & 0x0F;

    if (sn != isotp_expected_sn) {
      isotp_active = false;
      return false;
    }

    uint8_t copyLen = min((uint16_t)(len - 1), (uint16_t)(isotp_len - isotp_offset));
    memcpy(&isotp_buffer[isotp_offset], &data[1], copyLen);

    isotp_offset += copyLen;
    isotp_expected_sn++;

    if (isotp_offset >= isotp_len) {
      isotp_active = false;
      return true;
    }
  }

  return false;
}



void processCanMessage(long rxId, const uint8_t *rxBuf, uint8_t len) {

  // ======================================================
  // 1️⃣ MENSAGENS PROPRIETÁRIAS (PSA)
  // ======================================================
  for (size_t i = 0; i < NUM_CAN_SIGNALS; ++i) {
    const CanSignal* signal = &CAN_SIGNALS[i];

    if (signal->id == rxId) {
      last_prop_values[i] = decodeSignal(rxBuf, signal);
      return;
    }
  }


  // ======================================================
  // 2️⃣ RESPOSTAS OBD-II (ISO-TP)
  // ======================================================
  if (rxId < 0x7E8 || rxId > 0x7EF) return;

  obd_response_received = true;
  last_obd_response_time = millis();
  
  // Junta single / multi-frame
  if (!processIsoTpFrame(rxId, rxBuf, len)) {
    return;
  }

  // Payload ISO-TP completo
  uint8_t mode = isotp_buffer[0];
  uint8_t pid  = isotp_buffer[1];

  // ======================================================
  // 2.1️⃣ PID DISCOVERY
  // ======================================================
  if (mode == 0x41 &&
      (pid == 0x00 || pid == 0x20 || pid == 0x40)) {

    uint8_t basePid = pid;

    for (int byte = 0; byte < 4; byte++) {
      uint8_t mask = isotp_buffer[2 + byte];

      for (int bit = 0; bit < 8; bit++) {
        if (mask & (1 << (7 - bit))) {
          uint8_t supportedPid =
            basePid + (byte * 8) + bit + 1;
          supported_pids[supportedPid] = true;
        }
      }
    }

    Serial.print("PIDs suportados atualizados (base 0x");
    Serial.print(basePid, HEX);
    Serial.println(")");

    pid_discovery_index++;

    if (pid_discovery_index >=
        sizeof(pid_discovery_requests) /
        sizeof(pid_discovery_requests[0])) {

      pid_discovery_done = true;
      Serial.println("✅ PID discovery completo");
    }

    return;
  }

  // ======================================================
  // 2.2️⃣ OBD NORMAL
  // ======================================================
  if (mode != 0x41) return;

  for (size_t i = 0; i < NUM_OBD_PIDS; ++i) {

    const ObdPid* pid_map = &OBD_PIDS[i];

    if (pid_map->pid == pid) {
      last_obd_values[i] =
        decodeObdResponse(&isotp_buffer[2], pid_map);
      return;
    }
  }
}


// =================================================================
// LOOP PRINCIPAL
// =================================================================

void loop() {
  unsigned long currentMillis = millis();

if (currentMillis - last_obd_request_time >= obd_request_interval) {
    last_obd_request_time = currentMillis;

    // 🔁 Atualiza fallback de endereço (7DF → 7E0 → 7E1)
    updateObdAddressFallback();

    if (!pid_discovery_done) {

        // PID discovery (01 00, 01 20, 01 40)
        uint8_t req[3] = {
            0x02,           // número de bytes
            0x01,           // Mode 01
            pid_discovery_requests[pid_discovery_index]
        };

        sendObdWithFallback(req, 3);

    } else {

        // OBD normal
        const ObdPid* pid_map = &OBD_PIDS[current_obd_pid_index];

        uint8_t req[3] = {
            0x02,
            pid_map->mode,
            pid_map->pid
        };

        sendObdWithFallback(req, 3);

        current_obd_pid_index =
            (current_obd_pid_index + 1) % NUM_OBD_PIDS;
    }
}


  // 2. Leitura e Processamento de Mensagens CAN
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    long unsigned int rxId;
    unsigned char len = 0;
    unsigned char rxBuf[8];

    CAN.readMsgBuf(&rxId, &len, rxBuf);
    processCanMessage(rxId, rxBuf, len);
  }

  // 3. Exibição dos Dados Traduzidos
  if (currentMillis - last_print_time >= print_interval) {
    last_print_time = currentMillis;

    Serial.print("======================================\n");
    Serial.println("Scanner Completo - Dados Traduzidos:");
    Serial.println("--------------------------------------");

    // Exibe dados Proprietários (PSA)
    Serial.println("Dados Proprietários (PSA - Envio Contínuo):");
    for (size_t i = 0; i < NUM_CAN_SIGNALS; ++i) {
      const CanSignal* signal = &CAN_SIGNALS[i];
      Serial.print("  ");
      Serial.print(signal->name);
      Serial.print(": ");
      Serial.print(last_prop_values[i], 2);
      Serial.print(" ");
      Serial.println(signal->unit);
    }

    // Exibe dados OBD-II (Universal - Pedido/Resposta)
    Serial.println("--------------------------------------");
    Serial.println("Dados OBD-II (Universal - Pedido/Resposta):");
    for (size_t i = 0; i < NUM_OBD_PIDS; ++i) {
      const ObdPid* pid_map = &OBD_PIDS[i];
      Serial.print("  ");
      Serial.print(pid_map->name);
      Serial.print(": ");
      Serial.print(last_obd_values[i], 2);
      Serial.print(" ");
      Serial.println(pid_map->unit);
    }
    Serial.println("======================================");
  }

  delay(10);
}
