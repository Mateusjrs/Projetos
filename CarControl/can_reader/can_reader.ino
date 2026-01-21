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

char vehicleVin[18];   // 17 + '\0'
bool vinReceived = false;


uint8_t detectedCanSpeed = CAN_500KBPS;
bool canDetected = false;


// Definição dos pinos SPI para o ESP32
const int SPI_CS_PIN = 14;

// Inicializa o objeto MCP_CAN
MCP_CAN CAN(SPI_CS_PIN);

#define MAX_DTC 10

String dtcList[MAX_DTC];
uint8_t dtcCount = 0;

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


#define ISOTP_BUFFER_SIZE 64

struct IsoTpSession {
  uint16_t rxId;
  uint16_t txId;

  uint8_t buffer[ISOTP_BUFFER_SIZE];
  uint16_t length;
  uint16_t offset;

  uint8_t expected_sn;
  uint8_t block_size;
  uint8_t block_counter;
  uint8_t stmin;

  unsigned long lastFrameTime;
  bool active;
};

IsoTpSession isotp;
bool dtcReceived = false;


void requestVin() {
  memset(vehicleVin, 0, sizeof(vehicleVin));
  vinReceived = false;

  uint8_t req[3] = {
    0x02,
    0x09,
    0x02
  };

  sendObdWithFallback(req, 3);
}


enum ObdState {
  OBD_IDLE,
  OBD_REQUEST_VIN,
  OBD_REQUEST_DTC
};

ObdState obdState = OBD_IDLE;
bool obd_pid_received[NUM_OBD_PIDS];

void clearDtcList() {
  for (uint8_t i = 0; i < MAX_DTC; i++) {
    dtcList[i] = "";
  }
  dtcCount = 0;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  // --- Detecção de Velocidade (Mantém igual) ---
  for (size_t i = 0; i < sizeof(CAN_SPEEDS) / sizeof(CAN_SPEEDS[0]); i++) {
    if (tryCanSpeed(CAN_SPEEDS[i], CAN_SPEED_NAMES[i])) {
      detectedCanSpeed = CAN_SPEEDS[i];
      canDetected = true;
      break;
    }
  }

  if (!canDetected) {
    Serial.println("❌ Nenhuma velocidade CAN detectada");
    while (1); 
  }

  Serial.println("✅ CAN inicializado com sucesso");

  // ============================================================
  // 🚨 ALTERAÇÃO AQUI: REMOVENDO FILTROS RESTRITIVOS
  // ============================================================
  // Configura Máscaras para 0x00 (Aceita tudo, não filtra nada)
  // Mask 0 e Mask 1 cobrem todos os filtros
  CAN.init_Mask(0, 0, 0x00000000); 
  CAN.init_Mask(1, 0, 0x00000000);

  // Os filtros abaixo agora são irrelevantes por causa da máscara 0x00,
  // mas precisamos preenchê-los para o MCP não reclamar.
  // Colocamos 0 em tudo.
  for (int i = 0; i < 6; i++) {
    CAN.init_Filt(i, 0, 0x00000000);
  }
  
  Serial.println("🔓 Filtros desativados (Modo Promíscuo ativado)");

  memset(supported_pids, false, sizeof(supported_pids));
  memset(obd_pid_received, false, sizeof(obd_pid_received));
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

  const ObdPid* pid_map = &OBD_PIDS[current_obd_pid_index];

  // 1. Cria o pacote APENAS com os dados úteis (Modo + PID)
  // NÃO coloque o tamanho (0x02) aqui, a função sendIsoTp faz isso sozinha!
  uint8_t request_data[2] = {
    pid_map->mode,  // Mode 01
    pid_map->pid    // PID
  };

  // 2. Envia APENAS 2 bytes. 
  // Como 2 <= 7, a função sendIsoTp vai criar um "Single Frame" correto (0x02 M P ...)
  sendObdWithFallback(request_data, 2);

  Serial.print("📤 OBD Request: ");
  Serial.print(pid_map->name);
  Serial.print(" (0x");
  Serial.print(pid_map->pid, HEX);
  Serial.println(")");

  current_obd_pid_index = (current_obd_pid_index + 1) % NUM_OBD_PIDS;
}




void sendPidDiscoveryRequest() {

  if (pid_discovery_index >=
      sizeof(pid_discovery_requests) /
      sizeof(pid_discovery_requests[0])) {
    return;
  }

  uint8_t pid = pid_discovery_requests[pid_discovery_index];

  uint8_t req[8] = {
    0x02,       // número de bytes
    0x01,       // Mode 01
    pid,        // PID base (00, 20, 40)
    0, 0, 0, 0, 0
  };

  sendObdWithFallback(req, 8);

  Serial.print("PID discovery request: 01 ");
  Serial.println(pid, HEX);
}


unsigned long stminToMicros(uint8_t stmin) {
  if (stmin <= 0x7F) return stmin * 1000UL;
  if (stmin >= 0xF1 && stmin <= 0xF9)
    return (stmin - 0xF0) * 100UL;
  return 0;
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
void requestDTCs() {
  dtcReceived = false;
  clearDtcList();   // 🔹 limpa DTCs antigos

  // CORREÇÃO: O pedido de DTC é APENAS o byte do Modo (0x03).
  // Antes estava {0x01, 0x03}, o que confundia o simulador.
  uint8_t req[1] = {
    0x03 
  };

  // Envia apenas 1 byte. O sendIsoTp vai montar o frame "01 03 ..."
  sendObdWithFallback(req, 1);
  
  Serial.println("📤 Pedido de DTC (Mode 03) enviado...");
}




String decodeDtc(uint8_t a, uint8_t b) {
  if (a == 0 && b == 0) return "";

  char type;
  switch ((a >> 6) & 0x03) {
    case 0: type = 'P'; break;
    case 1: type = 'C'; break;
    case 2: type = 'B'; break;
    case 3: type = 'U'; break;
  }

  uint16_t code =
    ((a & 0x3F) << 8) | b;

  char buffer[6];
  snprintf(buffer, sizeof(buffer), "%c%04X", type, code);

  return String(buffer);
}



bool processIsoTpFrame(long rxId, const uint8_t *data, uint8_t len) {

  uint8_t pciType = data[0] >> 4;

  // ===============================
  // SINGLE FRAME
  // ===============================
  if (pciType == 0x0) {
    uint8_t payloadLen = data[0] & 0x0F;

    memcpy(isotp.buffer, &data[1], payloadLen);

    isotp.length = payloadLen;
    isotp.offset = payloadLen;
    isotp.active = false;

    return true;
  }

  // ===============================
  // FIRST FRAME
  // ===============================
  if (pciType == 0x1) {

    isotp.rxId = rxId;
    isotp.txId = rxId - 0x8;

    isotp.length = ((data[0] & 0x0F) << 8) | data[1];
    isotp.offset = len - 2;

    memcpy(isotp.buffer, &data[2], isotp.offset);

    isotp.expected_sn  = 1;
    isotp.block_size   = 0;
    isotp.block_counter = 0;
    isotp.stmin        = 0;
    isotp.lastFrameTime = millis();
    isotp.active       = true;

    // FLOW CONTROL - CTS
    uint8_t fc[8] = {
      0x30,  // CTS
      0x00,  // Block Size = ilimitado
      0x02,  // STmin = 5 ms
      0, 0, 0, 0, 0
    };

    CAN.sendMsgBuf(isotp.txId, 0, 8, fc);

    return false;
  }

  // ===============================
  // CONSECUTIVE FRAME
  // ===============================
  if (pciType == 0x2 && isotp.active) {

    uint8_t sn = data[0] & 0x0F;

    if (sn != isotp.expected_sn) {
      isotp.active = false;
      return false;
    }

    uint16_t remaining = isotp.length - isotp.offset;
    uint8_t copyLen = min((uint16_t)(len - 1), remaining);

    if (isotp.offset + copyLen > ISOTP_BUFFER_SIZE) {
      isotp.active = false;
      return false;
    }

    memcpy(&isotp.buffer[isotp.offset], &data[1], copyLen);
    isotp.offset += copyLen;

    isotp.expected_sn = (isotp.expected_sn + 1) & 0x0F;
    isotp.lastFrameTime = millis();

    if (isotp.offset >= isotp.length) {
      isotp.active = false;
      return true;
    }

    return false;
  }

  return false;
}


void processCanMessage(long rxId, const uint8_t *rxBuf, uint8_t len) {

  // ======================================================
  // 1️⃣ MENSAGENS PROPRIETÁRIAS (PSA)
  // ======================================================

  if (rxId >= 0x7E8 && rxId <= 0x7EF) {
     Serial.print("👀 RESPOSTA OBD DETECTADA! ID: 0x");
     Serial.print(rxId, HEX);
     Serial.print(" Dados: ");
     for(int k=0; k<len; k++){
        Serial.print(rxBuf[k], HEX);
        Serial.print(" ");
     }
     Serial.println();
  }
  //
  
  for (size_t i = 0; i < NUM_CAN_SIGNALS; ++i) {
    const CanSignal* signal = &CAN_SIGNALS[i];

    if (signal->id == rxId) {
      last_prop_values[i] = decodeSignal(rxBuf, signal);
      return;
    }
  }

  Serial.print("RX CAN ID: 0x");
  Serial.println(rxId, HEX);

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
  uint8_t mode = isotp.buffer[0];
  uint8_t pid  = isotp.buffer[1];
  uint8_t responseMode = isotp.buffer[0];
  uint8_t service = responseMode & 0x3F;  // remove 0x40

  // ======================================================
  // MODE 09 - VIN
  // ======================================================
  if (mode == 0x49 && pid == 0x02) {

    uint8_t vinIndex = isotp.buffer[2]; // índice do frame (01, 02, 03...)
    uint8_t dataStart = 3;

    for (uint8_t i = dataStart; i < isotp.length; i++) {
      if (isotp.buffer[i] >= 32 && isotp.buffer[i] <= 126) {
        size_t pos = strlen(vehicleVin);
        if (pos < 17) {
          vehicleVin[pos] = (char)isotp.buffer[i];
          vehicleVin[pos + 1] = '\0';
        }
      }
    }

    if (strlen(vehicleVin) >= 17) {
      vinReceived = true;
      Serial.print("✅ VIN recebido: ");
      Serial.println(vehicleVin);
    }

    return;
  }

  // ======================================================
  // MODE 03 - DTCs (Falhas)
  // ======================================================
  if (mode == 0x43) {  // 0x40 + 0x03
    dtcReceived = true;

    Serial.println("🚨 Falhas encontradas:");

    uint8_t start = (isotp.buffer[1] == 0x00) ? 2 : 1;

    for (uint8_t i = start; i + 1 < isotp.length; i += 2) {

      uint8_t a = isotp.buffer[i];
      uint8_t b = isotp.buffer[i + 1];

      String dtc = decodeDtc(a, b);

      if (dtc.length() > 0) {

        bool exists = false;
        for (uint8_t j = 0; j < dtcCount; j++) {
          if (dtcList[j] == dtc) {
            exists = true;
            break;
          }
        }

        if (!exists && dtcCount < MAX_DTC) {
          dtcList[dtcCount++] = dtc;
        }
      }

    }

    return;
  }


  // ======================================================
  // 2.1️⃣ PID DISCOVERY
  // ======================================================
  if (mode == 0x41 &&
      (pid == 0x00 || pid == 0x20 || pid == 0x40)) {

    uint8_t basePid = pid;

    for (int byte = 0; byte < 4; byte++) {
      uint8_t mask = isotp.buffer[2 + byte];

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
    /*
        pid_discovery_index++;

        if (pid_discovery_index >=
            sizeof(pid_discovery_requests) /
            sizeof(pid_discovery_requests[0])) {

          pid_discovery_done = true;
          Serial.println("✅ PID discovery completo");
        }
    */
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
        decodeObdResponse(&isotp.buffer[2], pid_map);

      obd_pid_received[i] = true;
      return;
    }

  }
}

// =================================================================
// FUNÇÃO AUXILIAR: EXIBIÇÃO DE DADOS NO SERIAL
// =================================================================
void printData() {
  
  // 1. Exibe DTCs (Códigos de Falha) se houver
  if (dtcCount > 0) {
    Serial.println("🚨 DTCs armazenados:");
    for (uint8_t i = 0; i < dtcCount; i++) {
      Serial.print("  - ");
      Serial.println(dtcList[i]);
    }
  } else {
    Serial.println("✅ Nenhuma falha armazenada");
  }

  Serial.println("======================================");
  Serial.println("Scanner Completo - Dados Traduzidos:");
  Serial.println("--------------------------------------");

  // 2. Exibe dados Proprietários (PSA)
  Serial.println("Dados Proprietários (PSA - Envio Contínuo):");
  for (size_t i = 0; i < NUM_CAN_SIGNALS; ++i) {
    const CanSignal* signal = &CAN_SIGNALS[i];
    
    Serial.print("  ");
    Serial.print(signal->name);
    Serial.print(": ");
    
    // Verifica se o valor é válido (opcional, mas bom pra debug)
    if (last_prop_values[i] == 0 && strcmp(signal->name, "Engine RPM") == 0) {
       // Se quiser tratar zeros específicos, pode fazer aqui
    }

    Serial.print(last_prop_values[i], 2);
    Serial.print(" ");
    Serial.println(signal->unit);
  }

  Serial.println("--------------------------------------");

  // 3. Exibe dados OBD-II (Universal)
  Serial.println("Dados OBD-II (Universal - Pedido/Resposta):");
  for (size_t i = 0; i < NUM_OBD_PIDS; ++i) {
    const ObdPid* pid_map = &OBD_PIDS[i];
    
    Serial.print("  ");
    Serial.print(pid_map->name);
    Serial.print(": ");
    
    // Mostra o valor armazenado
    Serial.print(last_obd_values[i], 2);
    Serial.print(" ");
    Serial.println(pid_map->unit);
  }
  
  Serial.println("======================================");
}
// =================================================================
// LOOP PRINCIPAL
// =================================================================
// =================================================================
// LOOP PRINCIPAL CORRIGIDO
// =================================================================
// =================================================================
// LOOP PRINCIPAL (COM VERIFICAÇÃO PERIÓDICA DE FALHAS)
// =================================================================
void loop() {
  unsigned long currentMillis = millis();

  // 1. PRIORIDADE MÁXIMA: Ler o CAN
  while (CAN_MSGAVAIL == CAN.checkReceive()) {
    long unsigned int rxId;
    unsigned char len = 0;
    unsigned char rxBuf[8];
    CAN.readMsgBuf(&rxId, &len, rxBuf);
    processCanMessage(rxId, rxBuf, len);
  }

  if (isotp.active && currentMillis - isotp.lastFrameTime > 1000) {
    isotp.active = false;
  }

  // Fallback de endereço
  updateObdAddressFallback();


  // ======================================================
  // LÓGICA DE ESTADOS (STATE MACHINE)
  // ======================================================
  static unsigned long stateEntryTime = 0;
  static unsigned long lastDtcCheckTime = 0; // 🕒 Cronômetro para checar falhas
  const long dtcCheckInterval = 5000;        // Checa falhas a cada 5 segundos

  // --- 1. INICIALIZAÇÃO (Pede VIN uma vez) ---
  if (obdState == OBD_IDLE && currentMillis > 3000 && !vinReceived && stateEntryTime == 0) {
    Serial.println("⏳ Iniciando solicitação de VIN...");
    obdState = OBD_REQUEST_VIN;
    stateEntryTime = currentMillis;
    requestVin(); 
  }

  // --- 2. VERIFICAÇÃO CÍCLICA DE FALHAS (A cada 5s) ---
  // Só entra aqui se já passou o tempo E se não estamos ocupados com VIN
  if (obdState == OBD_IDLE && vinReceived && (currentMillis - lastDtcCheckTime > dtcCheckInterval)) {
    Serial.println("🔍 Verificação Periódica de Falhas (DTC)...");
    obdState = OBD_REQUEST_DTC;
    stateEntryTime = currentMillis;
    requestDTCs();
    lastDtcCheckTime = currentMillis; // Reseta o cronômetro
  }


  // --- ESTADO: LENDO VIN ---
  if (obdState == OBD_REQUEST_VIN) {
    if (vinReceived) {
       Serial.println("✅ VIN Concluído.");
       obdState = OBD_IDLE; // Vai para IDLE (o timer acima cuidará do DTC)
    } 
    else if (currentMillis - stateEntryTime > 3000) {
       Serial.println("⚠️ Timeout VIN. Desistindo.");
       obdState = OBD_IDLE; 
       stateEntryTime = 1; 
       // Se falhar o VIN, marcamos como recebido fake para permitir checar DTCs
       vinReceived = true; 
    }
  }

  // --- ESTADO: LENDO DTC ---
  if (obdState == OBD_REQUEST_DTC) {
    if (dtcReceived) {
       obdState = OBD_IDLE; // Volta a ler sensores
    }
    else if (currentMillis - stateEntryTime > 3000) {
       Serial.println("⚠️ Timeout DTC. Voltando...");
       obdState = OBD_IDLE;
    }
  }

  // ======================================================
  // PID DISCOVERY
  // ======================================================
  if (!pid_discovery_done && (currentMillis - last_obd_request_time > 500)) {
     sendPidDiscoveryRequest();
     pid_discovery_index++;
     last_obd_request_time = currentMillis;
     if (pid_discovery_index >= sizeof(pid_discovery_requests) / sizeof(pid_discovery_requests[0])) {
        pid_discovery_done = true;
        Serial.println("✅ PID discovery finalizado");
     }
  }

  // ======================================================
  // OBD-II NORMAL (SENSORES)
  // ======================================================
  if (pid_discovery_done && obdState == OBD_IDLE) {
     if (currentMillis - last_obd_request_time > obd_request_interval) {
        sendObdRequest();
        last_obd_request_time = currentMillis;
     }
  }

  // 3. Exibição dos Dados
  if (currentMillis - last_print_time >= print_interval) {
    last_print_time = currentMillis;
    printData(); 
  }
}
