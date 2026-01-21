// =================================================================
// CÓDIGO PARA O NÓ 1: SIMULADOR DE ECU (ECU FAKE) - VERSÃO COMPLETA
// HARDWARE: ESP32 + MCP2515 (ou TJA1050/VP230 como transceptor)
// IDE: Arduino IDE
// =================================================================

#include <SPI.h>
#include <mcp_can.h>
#include "can_map.h"

// Definição dos pinos SPI para o ESP32
const int SPI_CS_PIN = 14;

// Inicializa o objeto MCP_CAN
MCP_CAN CAN(SPI_CS_PIN);

#define ISOTP_TX_ID  0x7E8
#define ISOTP_RX_ID  0x7E0

uint8_t isotpBuf[64];
uint16_t isotpLen = 0;
uint8_t isotpSeq = 1;


// Variáveis de simulação
float current_rpm = 800.0;
float current_speed = 0.0;
float current_temp = 20.0;
float current_load = 0.0; // Novo PID simulado

float throttle = 0.0;   // 0–100 %
float brake    = 0.0;   // 0–100 %
int   gear     = 1;     // -1 = R, 0 = N, 1..5

enum IgnitionState {
  IGN_OFF,
  IGN_ON,
  ENGINE_ON
};

IgnitionState ignitionState = IGN_OFF;


unsigned long previousMillis = 0;
const long interval_prop = 50; // Intervalo de envio de mensagens proprietárias (20Hz)

struct DTC {
  uint16_t code;     // ex: 0x0301 → P0301
  bool active;
};

#define MAX_DTCS 5
DTC dtcs[MAX_DTCS];
bool checkEngine = false;


void setup() {
  Serial.begin(115200);

  for (int i = 0; i < MAX_DTCS; i++) {
    dtcs[i].active = false;
  }


  SPI.begin();

  while (CAN_OK != CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ)) {
    Serial.println("CAN BUS Shield init fail");
    delay(500);
  }
  // Configura um filtro para aceitar solicitações OBD-II (ID 0x7DF)
  // Máscara 0x7FF para IDs padrão (11-bit)
  // Aceita tudo (modo teste / simulador)
  CAN.init_Mask(0, 0, 0x000);
  CAN.init_Filt(0, 0, 0x000);


  CAN.setMode(MCP_NORMAL);
}

void setDTC(uint16_t code) {
  for (int i = 0; i < MAX_DTCS; i++) {
    if (!dtcs[i].active) {
      dtcs[i].code = code;
      dtcs[i].active = true;
      checkEngine = true;
      Serial.print("DTC ATIVO: P");
      Serial.println(code, HEX);
      return;
    }
  }
}

void clearDTCs() {
  for (int i = 0; i < MAX_DTCS; i++) {
    dtcs[i].active = false;
  }
  checkEngine = false;
  Serial.println("DTCs limpos");
}


unsigned long faultTimer = 0;
void evaluateFaults() {

  static unsigned long overTempTimer = 0;
  static unsigned long overRpmTimer  = 0;
  static unsigned long loadTimer     = 0;

  if (ignitionState != ENGINE_ON) {
    overTempTimer = overRpmTimer = loadTimer = 0;
    return;
  }

  // 🔥 Superaquecimento
  if (current_temp > 110.0) {
    if (overTempTimer == 0) overTempTimer = millis();
    if (millis() - overTempTimer > 3000)
      setDTC(0x0210);
  } else overTempTimer = 0;

  // 🚨 Overspeed
  if (current_rpm > 4500.0) {
    if (overRpmTimer == 0) overRpmTimer = millis();
    if (millis() - overRpmTimer > 2000)
      setDTC(0x0300);
  } else overRpmTimer = 0;

  // ⚠️ Carga alta em baixa velocidade
  if (throttle > 80.0 && current_speed < 10.0) {
    if (loadTimer == 0) loadTimer = millis();
    if (millis() - loadTimer > 4000)
      setDTC(0x0101);
  } else loadTimer = 0;
}



// =================================================================
// LÓGICA DE SIMULAÇÃO
// =================================================================
void isotpSend(uint16_t totalLen, uint8_t *payload) {

  // -------------------------------
  // SINGLE FRAME
  // -------------------------------
  if (totalLen <= 7) {
    uint8_t tx[8] = {0};
    tx[0] = totalLen;
    memcpy(&tx[1], payload, totalLen);
    CAN.sendMsgBuf(ISOTP_TX_ID, 0, 8, tx);
    return;
  }

  // -------------------------------
  // FIRST FRAME
  // -------------------------------
  uint8_t tx[8] = {0};
  tx[0] = 0x10 | ((totalLen >> 8) & 0x0F);
  tx[1] = totalLen & 0xFF;
  memcpy(&tx[2], payload, 6);
  CAN.sendMsgBuf(ISOTP_TX_ID, 0, 8, tx);

  delay(2); // tempo simples (scanner aceita)

  // -------------------------------
  // CONSECUTIVE FRAMES
  // -------------------------------
  uint16_t offset = 6;
  uint8_t seq = 1;

  while (offset < totalLen) {
    memset(tx, 0, 8);
    tx[0] = 0x20 | (seq & 0x0F);

    uint8_t chunk = min((uint16_t)7, (uint16_t)(totalLen - offset));
    memcpy(&tx[1], &payload[offset], chunk);

    CAN.sendMsgBuf(ISOTP_TX_ID, 0, 8, tx);

    offset += chunk;
    seq = (seq + 1) & 0x0F;
    delay(2);
  }
}


void simulateData() {

  // 🔴 Ignição desligada → carro morto
  if (ignitionState == IGN_OFF) {
    current_rpm   = 0.0;
    current_speed = 0.0;
    current_load  = 0.0;
    current_temp  = 20.0;
    return;
  }

  // 🟡 Ignição ligada, motor desligado
  if (ignitionState == IGN_ON) {
    current_rpm   = 0.0;
    current_speed = 0.0;
    current_load  = 0.0;

    if (current_temp < 30.0)
      current_temp += 0.05;

    return;
  }

  // 🟢 Motor ligado (simulação normal)
  if (ignitionState == ENGINE_ON) {

    // RPM base (marcha lenta)
    float target_rpm = 800.0 + throttle * 35.0;

    // Freio reduz RPM
    target_rpm -= brake * 15.0;

    // Limites
    if (target_rpm < 800.0) target_rpm = 800.0;
    if (target_rpm > 4500.0) target_rpm = 4500.0;

    // Suavização
    current_rpm += (target_rpm - current_rpm) * 0.1;

    // Velocidade depende da marcha
    if (gear > 0 && brake < 5.0) {
      current_speed += (throttle * gear * 0.01);
    }

    if (brake > 0.0) {
      current_speed -= brake * 0.05;
    }

    if (current_speed < 0) current_speed = 0;

    // Load baseado no throttle
    current_load = throttle;

    // Temperatura
    if (current_temp < 90.0)
      current_temp += 0.05;
  }


  if (current_speed < 0.0) current_speed = 0.0;
  if (current_load < 0.0)  current_load = 0.0;
  if (current_load > 100.) current_load = 100.0;
}

// =================================================================
// LÓGICA DE ENVIO PROPRIETÁRIO (PSA)
// =================================================================

void buildAndSendProprietaryFrame(long id) {
  uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  bool signal_encoded = false;

  for (size_t i = 0; i < NUM_CAN_SIGNALS; ++i) {
    const CanSignal* signal = &CAN_SIGNALS[i];

    if (signal->id == id) {
      float value_to_encode = 0.0;

      if (strcmp(signal->name, "Engine RPM (Prop)") == 0) {
        value_to_encode = current_rpm;
      } else if (strcmp(signal->name, "Vehicle Speed (Prop)") == 0) {
        value_to_encode = current_speed;
      } else if (strcmp(signal->name, "Coolant Temp (Prop)") == 0) {
        value_to_encode = current_temp;
      } else {
        continue;
      }

      if (encodeSignal(value_to_encode, signal, data)) {
        signal_encoded = true;
      }
    }
  }

  if (signal_encoded) {
    CAN.sendMsgBuf(id, 0, 8, data);
  }
}

// =================================================================
// LÓGICA DE RESPOSTA OBD-II
// =================================================================

void handleObdRequest(long rxId, const uint8_t *rxBuf, uint8_t len) {

  // Ignicao OFF → não responde nada
  if (ignitionState == IGN_OFF) return;

  // ID OBD padrão
  if ((rxId != 0x7DF && rxId != 0x7E0) || len < 2) return;
  // ✅ ID de resposta OBD (sempre ECU + 0x08)
  uint16_t responseId = 0x7E8;

  uint8_t mode = rxBuf[1];

  // ==================================================
  // MODE 03 → LEITURA DE DTC
  // ==================================================
  if (mode == 0x03) {

    uint8_t tx[8] = {0};
    tx[1] = 0x43;

    uint8_t idx = 2;

    for (int i = 0; i < MAX_DTCS && idx <= 6; i++) {
      if (dtcs[i].active) {
        tx[idx++] = (dtcs[i].code >> 8) & 0xFF;
        tx[idx++] = dtcs[i].code & 0xFF;
      }
    }

    tx[0] = idx - 1;  // bytes após byte 0

    CAN.sendMsgBuf(responseId, 0, 8, tx);

    return;
  }


  // ==================================================
  // MODE 04 → LIMPAR DTC
  // ==================================================
  if (mode == 0x04) {

    clearDTCs();

    uint8_t tx[8] = {0x01, 0x44, 0, 0, 0, 0, 0, 0};
    CAN.sendMsgBuf(responseId, 0, 8, tx);
    return;
  }
  // ==================================================
  // MODE 09 → VIN (OBD-II)
  // ==================================================
  if (mode == 0x09 && rxBuf[2] == 0x02) {

    uint8_t vinPayload[] = {
      0x49, 0x02, 0x01,
      '9', 'B', 'W', 'Z', 'Z', 'Z', '3', '7', '7', 'V', 'T', '0', '0', '0', '1'
    };

    isotpSend(sizeof(vinPayload), vinPayload);
    return;
  }


  // ==================================================
  // MODE 01 → DADOS EM TEMPO REAL (RPM, SPEED, ETC)
  // ==================================================
  if (mode != 0x01 || len < 3) return;

  uint8_t pid = rxBuf[2];  // ✅ DECLARA PRIMEIRO

  // ==================================================
  // MODE 01 PID 00 → Supported PIDs
  // ==================================================
  if (pid == 0x00) {
    uint8_t tx[8] = {
      0x06,
      0x41,
      0x00,
      0b10111110,
      0b00011111,
      0b10101000,
      0b00010011,
      0x00
    };
    CAN.sendMsgBuf(responseId, 0, 8, tx);
    return;
  }


  pid = rxBuf[2];

  for (size_t i = 0; i < NUM_OBD_PIDS; i++) {

    const ObdPid* pid_map = &OBD_PIDS[i];

    if (pid_map->pid == pid) {

      float value = 0.0;

      if (strcmp(pid_map->name, "Engine RPM (OBD)") == 0)
        value = current_rpm;
      else if (strcmp(pid_map->name, "Vehicle Speed (OBD)") == 0)
        value = current_speed;
      else if (strcmp(pid_map->name, "Coolant Temp (OBD)") == 0)
        value = current_temp;
      else if (strcmp(pid_map->name, "Calculated Load (OBD)") == 0)
        value = current_load;
      else if (strcmp(pid_map->name, "Intake Air Temp (OBD)") == 0)
        value = current_temp - 10.0;

      else if (strcmp(pid_map->name, "Fuel Level (OBD)") == 0)
        value = 75.0;  // simulado

      else if (strcmp(pid_map->name, "Ambient Air Temp (OBD)") == 0)
        value = current_temp - 15.0;

      else if (strcmp(pid_map->name, "Run Time Since Start (OBD)") == 0)
        value = millis() / 1000;

      else if (strcmp(pid_map->name, "Intake MAP (OBD)") == 0)
        value = throttle * 1.2; // kPa fake

      else if (strcmp(pid_map->name, "MAF Air Flow (OBD)") == 0)
        value = current_rpm * throttle / 1000.0;

      else if (strcmp(pid_map->name, "Battery Voltage (OBD)") == 0)
        value = 13.8;

      else
        return;

      uint8_t txBuf[8] = {0};

      txBuf[0] = 2 + pid_map->bytes_in_resp;
      txBuf[1] = 0x41;
      txBuf[2] = pid;

      encodeObdResponse(value, pid_map, &txBuf[3]);

      CAN.sendMsgBuf(0x7E8, 0, 8, txBuf); 

      return;
    }
  }
}


void handleIgnitionSerial() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  if (cmd == "IGN OFF") {
    ignitionState = IGN_OFF;
    throttle = 0;
    brake = 0;
    gear = 1;
    clearDTCs();
    Serial.println("Ignição OFF");
  }

  else if (cmd == "IGN ON") {
    ignitionState = IGN_ON;
    Serial.println("Ignição ON");
  }
  else if (cmd == "ENGINE ON") {
    ignitionState = ENGINE_ON;
    Serial.println("Motor LIGADO");
  }
  else if (cmd == "ENGINE OFF") {
    ignitionState = IGN_ON;
    Serial.println("Motor DESLIGADO");
  }
}


void handleCarControlSerial() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd.startsWith("THROTTLE")) {
    throttle = cmd.substring(9).toFloat();
    throttle = constrain(throttle, 0, 100);
    Serial.println("Throttle setado");
  }

  else if (cmd.startsWith("BRAKE")) {
    brake = cmd.substring(6).toFloat();
    brake = constrain(brake, 0, 100);
    Serial.println("Brake setado");
  }

  else if (cmd.startsWith("GEAR")) {
    gear = cmd.substring(5).toInt();
    Serial.println("Marcha alterada");
  }
}

void handleUdsRequest(long rxId, const uint8_t *rxBuf, uint8_t len) {

  if (rxId != 0x7E0 || len < 3) return;

  uint8_t sid = rxBuf[0];

  uint16_t did = (rxBuf[1] << 8) | rxBuf[2];

  // ==================================================
  // 0x22 → ReadDataByIdentifier
  // ==================================================
  if (sid == 0x22) {

    // ================================
    // VIN – DID 0xF190 (ISO-TP)
    // ================================
    if (did == 0xF190) {

      // Payload UDS completo (resposta positiva)
      uint8_t vinPayload[] = {
        0x62, 0xF1, 0x90,
        '9', 'B', 'W', 'Z', 'Z', 'Z', '3', '7', '7', 'V', 'T', '0', '0', '0', '1'
      };

      isotpSend(sizeof(vinPayload), vinPayload);
      return;
    }

    // ================================
    // OUTROS DIDs CURTOS (frame único)
    // ================================
    uint8_t tx[8] = {0};
    tx[0] = 0x62;
    tx[1] = rxBuf[1];
    tx[2] = rxBuf[2];

    if (did == 0xF191) { // Speed
      tx[3] = (uint8_t)current_speed;
      CAN.sendMsgBuf(0x7E8, 0, 4, tx);
    }

    else if (did == 0xF192) { // Throttle
      tx[3] = (uint8_t)throttle;
      CAN.sendMsgBuf(0x7E8, 0, 4, tx);
    }

    else if (did == 0xF193) { // Gear
      tx[3] = (int8_t)gear;
      CAN.sendMsgBuf(0x7E8, 0, 4, tx);
    }

    else if (did == 0xF194) { // Temp
      tx[3] = (uint8_t)current_temp;
      CAN.sendMsgBuf(0x7E8, 0, 4, tx);
    }

    return;
  }


  // ==================================================
  // 0x2E → WriteDataByIdentifier
  // ==================================================
  if (sid == 0x2E && len >= 5) {

    uint16_t value = (rxBuf[3] << 8) | rxBuf[4];

    if (did == 0xF192) { // Throttle
      throttle = constrain(value, 0, 100);
    }

    else if (did == 0xF193) { // Gear
      gear = (int8_t)value;
    }

    uint8_t tx[8] = {0x6E, rxBuf[1], rxBuf[2], 0, 0, 0, 0, 0};
    CAN.sendMsgBuf(0x7E8, 0, 3, tx);
    return;
  }
}

void handleSerialCommand() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.replace("\r", "");
  cmd.trim();

  Serial.print("CMD: ");
  Serial.println(cmd);

  if (cmd.equalsIgnoreCase("IGN OFF")) {
    ignitionState = IGN_OFF;
    throttle = 0;
    brake = 0;
    gear = 1;
    clearDTCs();
    Serial.println("Ignição OFF");
  }
  else if (cmd.equalsIgnoreCase("IGN ON")) {
    ignitionState = IGN_ON;
    Serial.println("Ignição ON");
  }
  else if (cmd.equalsIgnoreCase("ENGINE ON")) {
    ignitionState = ENGINE_ON;
    Serial.println("Motor LIGADO");
  }
  else if (cmd.equalsIgnoreCase("ENGINE OFF")) {
    ignitionState = IGN_ON;
    Serial.println("Motor DESLIGADO");
  }
  else if (cmd.startsWith("THROTTLE ")) {
    throttle = constrain(cmd.substring(9).toFloat(), 0, 100);
    Serial.println("Throttle setado");
  }
  else if (cmd.startsWith("BRAKE ")) {
    brake = constrain(cmd.substring(6).toFloat(), 0, 100);
    Serial.println("Brake setado");
  }
  else if (cmd.startsWith("GEAR ")) {
    gear = cmd.substring(5).toInt();
    Serial.println("Marcha alterada");
  }
  else if (cmd.startsWith("UDS ")) {
    uint8_t data[8];
    int len = 0;

    // Pega a parte após "UDS "
    String payload = cmd.substring(4);

    // strtok NÃO funciona direto com String → precisa de buffer
    char buffer[32];
    payload.toCharArray(buffer, sizeof(buffer));

    char *token = strtok(buffer, " ");
    while (token && len < 8) {
      data[len++] = strtol(token, NULL, 16);
      token = strtok(NULL, " ");
    }

    CAN.sendMsgBuf(0x7E0, 0, len, data);
    Serial.println("UDS enviado");
  }

  else {
    Serial.println("Comando desconhecido");
  }
}

// =================================================================
// LOOP PRINCIPAL
// =================================================================
void loop() {

  unsigned long now = millis();

  if (now - previousMillis >= interval_prop) {
    previousMillis = now;

    simulateData();
    handleSerialCommand();

    evaluateFaults();

    if (ignitionState == ENGINE_ON) {
      buildAndSendProprietaryFrame(0x0B6);
      buildAndSendProprietaryFrame(0x0F6);
    }

  }

  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    long unsigned int rxId;
    unsigned char len;
    unsigned char rxBuf[8];

    CAN.readMsgBuf(&rxId, &len, rxBuf);

    handleObdRequest(rxId, rxBuf, len);
    handleUdsRequest(rxId, rxBuf, len);
  }

}
