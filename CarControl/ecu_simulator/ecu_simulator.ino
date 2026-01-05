// =================================================================
// CÓDIGO PARA O NÓ 1: SIMULADOR DE ECU (ECU FAKE) - VERSÃO COMPLETA
// HARDWARE: ESP32 + MCP2515 (ou TJA1050/VP230 como transceptor)
// IDE: Arduino IDE
// =================================================================

#include <SPI.h>
#include <mcp_can.h>
#include "can_map.h"

// Definição dos pinos SPI para o ESP32
const int SPI_CS_PIN = 5; 

// Inicializa o objeto MCP_CAN
MCP_CAN CAN(SPI_CS_PIN);

// Variáveis de simulação
float current_rpm = 800.0;
float current_speed = 0.0;
float current_temp = 20.0;
float current_load = 0.0; // Novo PID simulado

unsigned long previousMillis = 0;
const long interval_prop = 50; // Intervalo de envio de mensagens proprietárias (20Hz)

void setup() {
    Serial.begin(115200);
    
    // Inicializa o CAN bus a 500kbps
    while (CAN_OK != CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ)) {
        Serial.println("CAN BUS Shield init fail");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok! (Simulador Completo)");
    
    // Configura um filtro para aceitar solicitações OBD-II (ID 0x7DF)
    // Máscara 0x7FF para IDs padrão (11-bit)
    CAN.init_Mask(0, 0, 0x7FF); 
    // Filtro 0: Aceita apenas solicitações OBD-II (0x7DF)
    CAN.init_Filt(0, 0, CAN_ID_OBD_REQUEST); 
    
    CAN.setMode(MCP_NORMAL);
}

// =================================================================
// LÓGICA DE SIMULAÇÃO
// =================================================================

void simulateData() {
    // Simula RPM e Velocidade
    if (current_rpm < 4000.0 && current_speed < 100.0) {
        current_rpm += 25.0;
        current_speed += 0.5;
        current_load += 0.5;
    } else if (current_rpm >= 4000.0) {
        current_rpm = 3975.0; 
        current_load = 70.0;
    } else if (current_rpm > 800.0) {
        current_rpm -= 25.0;
        current_speed -= 0.5;
        current_load -= 0.5;
    } else {
        current_rpm = 800.0;
        current_speed = 0.0;
        current_load = 0.0;
    }

    // Simula Temperatura
    if (current_temp < 90.0) {
        current_temp += 0.1;
    } else if (current_temp > 90.0) {
        current_temp = 90.0;
    }
    
    if (current_speed < 0.0) current_speed = 0.0;
    if (current_load < 0.0) current_load = 0.0;
    if (current_load > 100.0) current_load = 100.0;

    Serial.print("Simulando: RPM=");
    Serial.print(current_rpm, 1);
    Serial.print(", Speed=");
    Serial.print(current_speed, 1);
    Serial.print(" km/h, Temp=");
    Serial.print(current_temp, 1);
    Serial.print(" C, Load=");
    Serial.print(current_load, 1);
    Serial.println(" %");
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

    // Espera no mínimo: [numBytes][mode][pid]
    if (rxId != CAN_ID_OBD_REQUEST || len < 3) return;

    uint8_t numBytes = rxBuf[0];
    uint8_t mode     = rxBuf[1];
    uint8_t pid      = rxBuf[2];

    Serial.print("OBD Request: Mode=0x");
    Serial.print(mode, HEX);
    Serial.print(" PID=0x");
    Serial.println(pid, HEX);

    // Apenas modo 01
    if (mode != 0x01) return;

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
            else
                return;

            uint8_t txBuf[8] = {0};

            // Byte 0 → número de bytes após ele
            txBuf[0] = 2 + pid_map->bytes_in_resp;
            txBuf[1] = mode + 0x40;   // 0x41
            txBuf[2] = pid;

            encodeObdResponse(value, pid_map, &txBuf[3]);

            // DLC CAN = 1 + bytes indicados
            uint8_t canLen = txBuf[0] + 1;

            CAN.sendMsgBuf(CAN_ID_OBD_RESPONSE, 0, canLen, txBuf);

            Serial.print("OBD Response enviada: ");
            Serial.println(pid_map->name);
            return;
        }
    }

    // PID não suportado → resposta negativa
    uint8_t nrc[8] = {0x03, 0x7F, mode, 0x12, 0, 0, 0, 0};
    CAN.sendMsgBuf(CAN_ID_OBD_RESPONSE, 0, 4, nrc);
}

// =================================================================
// LOOP PRINCIPAL
// =================================================================
void loop() {

    unsigned long now = millis();

    if (now - previousMillis >= interval_prop) {
        previousMillis = now;

        simulateData();
        buildAndSendProprietaryFrame(0x0B6);
        buildAndSendProprietaryFrame(0x0F6);
    }

    if (CAN_MSGAVAIL == CAN.checkReceive()) {
        long unsigned int rxId;
        unsigned char len;
        unsigned char rxBuf[8];

        CAN.readMsgBuf(&rxId, &len, rxBuf);

        handleObdRequest(rxId, rxBuf, len);
    }
}
