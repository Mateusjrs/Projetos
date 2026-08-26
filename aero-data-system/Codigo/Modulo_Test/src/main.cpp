// OS leds 0, 1 e 2 do PCF 0x27,porem o 2 não acende

#include <Arduino.h>
#include <Wire.h>
#include <Multiplexador.h>

// ESP32-C3 Super Mini
// Leitura de todas as entradas analógicas

const uint8_t adcPins[] = {0, 1, 2, 3, 4};
const uint8_t numPins = sizeof(adcPins) / sizeof(adcPins[0]);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Configura resolução do ADC
  analogReadResolution(12); // 0 a 4095

  Serial.println("Leitura das entradas analógicas:");
}

void loop() {

  for (uint8_t i = 0; i < numPins; i++) {

    int valor = analogRead(adcPins[i]);

    Serial.print("GPIO ");
    Serial.print(adcPins[i]);
    Serial.print(" = ");
    Serial.print(valor);

    float tensao = valor * (3.3 / 4095.0);

    Serial.print("  (");
    Serial.print(tensao, 3);
    Serial.println(" V)");
  }

  Serial.println("---------------------------");

  delay(500);
}