#ifndef CAN_MAP_H
#define CAN_MAP_H

#include <Arduino.h>
#include <mcp_can.h>

// =================================================================
// CONSTANTES GERAIS
// =================================================================
#define CAN_ID_OBD_REQUEST  0x7DF // ID de solicitação broadcast (para todos)
#define CAN_ID_OBD_RESPONSE 0x7E8 // ID de resposta da ECU (ECU 0)

// =================================================================
// ESTRUTURA DE DADOS GENÉRICA PARA SINAIS CAN (PROPRIETÁRIO)
// =================================================================

enum SignalType {
    TYPE_UNSIGNED,
    TYPE_SIGNED,
    TYPE_FLOAT
};

// Estrutura para mapear um único sinal dentro de uma mensagem CAN proprietária
typedef struct {
    const char* name;
    long id;
    uint8_t start_byte;
    uint8_t start_bit;
    uint8_t length;
    float factor;
    float offset;
    const char* unit;
    SignalType type;
} CanSignal;


// =================================================================
// ESTRUTURA DE DADOS PARA PIDs OBD-II (SAE J1979)
// =================================================================

// Estrutura para mapear um PID OBD-II
typedef struct {
    const char* name;       // Nome do PID (ex: "Engine RPM")
    uint8_t mode;           // Modo de serviço (ex: 0x01)
    uint8_t pid;            // PID (ex: 0x0C)
    uint8_t bytes_in_resp;  // Quantidade de bytes de dados na resposta (ex: 2 para RPM)
    float factor;           // Fator de escala (ex: 0.25)
    float offset;           // Offset (ex: -40)
    const char* unit;       // Unidade (ex: "RPM", "km/h", "C")
} ObdPid;

// =================================================================
// MAPA DE SINAIS CAN PROPRIETÁRIOS (PSA)
// =================================================================

const CanSignal CAN_SIGNALS[] = {
    // 1. Sinais de Motor/Velocidade (ID 0x0B6) - BSI RPM/Speed
    { "Engine RPM (Prop)", 0x0B6, 2, 0, 16, 0.25, 0.0, "RPM", TYPE_FLOAT },
    { "Vehicle Speed (Prop)", 0x0B6, 4, 0, 8, 1.0, 0.0, "km/h", TYPE_UNSIGNED },
    // 2. Sinais de Temperatura (ID 0x0F6)
    { "Coolant Temp (Prop)", 0x0F6, 1, 0, 8, 1.0, -40.0, "C", TYPE_SIGNED },
};

const size_t NUM_CAN_SIGNALS = sizeof(CAN_SIGNALS) / sizeof(CAN_SIGNALS[0]);

// =================================================================
// MAPA DE PIDs OBD-II (Modo 01 - Dados de Tempo Real)
// =================================================================

const ObdPid OBD_PIDS[] = {

  // =====================
  // BÁSICOS (já tinha)
  // =====================
  { "Calculated Load (OBD)",      0x01, 0x04, 1, 100.0 / 255.0, 0.0, "%" },
  { "Coolant Temp (OBD)",         0x01, 0x05, 1, 1.0, -40.0, "C" },
  { "Engine RPM (OBD)",           0x01, 0x0C, 2, 0.25, 0.0, "RPM" },
  { "Vehicle Speed (OBD)",        0x01, 0x0D, 1, 1.0, 0.0, "km/h" },
  { "Throttle Position (OBD)",    0x01, 0x11, 1, 100.0 / 255.0, 0.0, "%" },

  // =====================
  // NOVOS PIDs (UNIVERSAIS)
  // =====================
  { "Intake Air Temp (OBD)",      0x01, 0x0F, 1, 1.0, -40.0, "C" },
  { "Fuel Level (OBD)",           0x01, 0x2F, 1, 100.0 / 255.0, 0.0, "%" },
  { "Ambient Air Temp (OBD)",     0x01, 0x46, 1, 1.0, -40.0, "C" },
  { "Run Time Since Start (OBD)", 0x01, 0x1F, 2, 1.0, 0.0, "s" },
  { "Intake MAP (OBD)",           0x01, 0x0B, 1, 1.0, 0.0, "kPa" },
  { "MAF Air Flow (OBD)",          0x01, 0x10, 2, 1.0 / 100.0, 0.0, "g/s" },
  { "Battery Voltage (OBD)",      0x01, 0x42, 2, 1.0 / 1000.0, 0.0, "V" },
};


const size_t NUM_OBD_PIDS = sizeof(OBD_PIDS) / sizeof(OBD_PIDS[0]);

// =================================================================
// FUNÇÕES GENÉRICAS DE CODIFICAÇÃO E DECODIFICAÇÃO (PROPRIETÁRIO)
// =================================================================

// Função de decodificação proprietária (mantida do código anterior)
float decodeSignal(const uint8_t *data, const CanSignal *signal) {
    uint64_t raw_value = 0;
    
    if (signal->length == 8) {
        raw_value = data[signal->start_byte];
    } else if (signal->length == 16) {
        raw_value = ((uint16_t)data[signal->start_byte] << 8) | data[signal->start_byte + 1];
    } else {
        return NAN;
    }

    float decoded_value = (float)raw_value * signal->factor + signal->offset;
    return decoded_value;
}

// Função de codificação proprietária (mantida do código anterior)
bool encodeSignal(float value, const CanSignal *signal, uint8_t *data) {
    uint64_t raw_value = (uint64_t)((value - signal->offset) / signal->factor);

    if (signal->length == 8) {
        data[signal->start_byte] = (uint8_t)raw_value;
    } else if (signal->length == 16) {
        data[signal->start_byte] = (uint8_t)(raw_value >> 8);
        data[signal->start_byte + 1] = (uint8_t)(raw_value & 0xFF);
    } else {
        return false;
    }
    
    return true;
}

// =================================================================
// FUNÇÕES DE CODIFICAÇÃO E DECODIFICAÇÃO OBD-II
// =================================================================

/**
 * @brief Decodifica o valor de um PID OBD-II a partir dos bytes de resposta.
 * @param response_data Bytes de dados da resposta OBD-II (a partir do byte 3).
 * @param pid_map Estrutura ObdPid com o mapeamento do PID.
 * @return Valor decodificado (float).
 */
float decodeObdResponse(const uint8_t *response_data, const ObdPid *pid_map) {
    uint32_t raw_value = 0;
    
    // A resposta OBD-II começa no byte 3 da mensagem CAN (após DLC, Mode+0x40, PID)
    // response_data[0] é o primeiro byte de dados (A)
    
    if (pid_map->bytes_in_resp == 1) {
        raw_value = response_data[0];
    } else if (pid_map->bytes_in_resp == 2) {
        // Assume Big-Endian (Byte A, Byte B)
        raw_value = ((uint32_t)response_data[0] << 8) | response_data[1];
    } else {
        // PIDs mais complexos (3 ou 4 bytes)
        return NAN;
    }

    // Aplica o fator de escala e offset
    return (float)raw_value * pid_map->factor + pid_map->offset;
}

/**
 * @brief Codifica o valor de um PID OBD-II nos bytes de resposta.
 * @param value Valor a ser codificado.
 * @param pid_map Estrutura ObdPid com o mapeamento do PID.
 * @param response_data Buffer para os bytes de dados da resposta (a partir do byte 3).
 * @return true se a codificação foi bem-sucedida.
 */
bool encodeObdResponse(float value, const ObdPid *pid_map, uint8_t *response_data) {
    uint32_t raw_value = (uint32_t)((value - pid_map->offset) / pid_map->factor);

    if (pid_map->bytes_in_resp == 1) {
        response_data[0] = (uint8_t)raw_value;
    } else if (pid_map->bytes_in_resp == 2) {
        // Assume Big-Endian (Byte A, Byte B)
        response_data[0] = (uint8_t)(raw_value >> 8);
        response_data[1] = (uint8_t)(raw_value & 0xFF);
    } else {
        return false;
    }
    
    return true;
}

#endif // CAN_MAP_H
