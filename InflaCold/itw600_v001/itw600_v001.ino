/*
  v000 – Created a new project scope for ITW-600.
  v001 – Started testing with basic control, supporting only Samsung, Copix, and Hitachi. During the Hitachi A/C tests, difficulties were found in identifying and matching the correct A/C models.
  v002 – Added a new search and matching method, along with an automatic detection mechanism to set the correct control parameters.Added controled temp
  v003 – Added wifi and server mqtt. Created the payload scheth 
*/

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>    

#include <ir_Samsung.h>
#include <ir_Carrier.h>
#include <ir_Midea.h>
#include <ir_Gree.h>
#include <ir_Hitachi.h>
#include <ir_Coolix.h>
#include <ir_Panasonic.h>
#include <ir_Fujitsu.h>
#include <ir_Bosch.h>
#include <ir_Kelon.h>
#include <ir_Mitsubishi.h>
#include <ir_Toshiba.h>


#include <OneWire.h>
#include <DallasTemperature.h>

#include <ServerHandle.h>
#include <memoryManagment.h>
#include <WebServer.h>
#include <PayloadItw600.h>
#include <WiFiManager.h>
#include <WifiHandle.h>
#include "sdkconfig.h"
#include "esp_idf_version.h"
#include <WiFi.h>
#include <EEPROM.h>


// --- PINOS ---
#define PIN_DS18B20  26  // Exemplo: Pino do Sensor de Temperatura
#define PIN_LED_R    12 // LED Vermelho
#define PIN_LED_G    13 // LED Verde
#define PIN_LED_B    14 // LED Azul

// --- OBJETOS DO SENSOR ---
OneWire oneWire(PIN_DS18B20);
DallasTemperature sensors(&oneWire);

// --- VARIÁVEIS GLOBAIS NOVAS ---
float targetTemp = 24;          // Temperatura alvo (padrão 24)
bool thermostatEnabled = false; // Se o modo automático está ligado
unsigned long lastTempCheck = 0;
bool acIsOn = false;          // Para saber o estado atual e não ficar enviando comando repetido
int ConnectCont = 0;

TaskHandle_t SwicthPayloadHandle;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

String ssid ;;
String password;

float T_set = 23.0;

float hysteresis = 0.5;
float overshoot_margin = 0.3;

unsigned long min_action_time = 60000; // 3 min
unsigned long settling_time   = 60000; // 3 min

float alpha = 0.1;

float T_raw = 0;
float T_filt = 0;
float T_prev = 0;

unsigned long last_action_time = 0;
unsigned long last_read_time = 0;

float dTdt = 0;

uint8_t AC_setpoint = 25;
const uint8_t AC_MIN_TEMP = 16;
const uint8_t AC_MAX_TEMP = 30;





// ============================ PINOS ============================
const uint16_t IR_SEND_PIN = 5;
const uint16_t IR_RECV_PIN = 23;



// ============================ INSTÂNCIAS ============================
IRSamsungAc       acSamsung(IR_SEND_PIN);
IRCarrierAc64     acCarrier(IR_SEND_PIN);
IRMideaAC         acMidea(IR_SEND_PIN);
IRGreeAC          acGree(IR_SEND_PIN);
IRHitachiAc       acHitachi(IR_SEND_PIN);
IRHitachiAc1      acHitachi1(IR_SEND_PIN);
IRHitachiAc264    acHitachi264(IR_SEND_PIN);
IRHitachiAc344    acHitachi344(IR_SEND_PIN);
IRHitachiAc424    acHitachi424(IR_SEND_PIN);
IRHitachiAc296    acHitachi296(IR_SEND_PIN);
IRCoolixAC        acCoolix(IR_SEND_PIN);
IRPanasonicAc     acPanasonic(IR_SEND_PIN);
IRPanasonicAc32   acPanasonic32(IR_SEND_PIN);
IRFujitsuAC       acFujitsu(IR_SEND_PIN);
IRBosch144AC      acBosch(IR_SEND_PIN);
IRKelonAc         acKelon(IR_SEND_PIN);
IRMitsubishiAC    acMitsubishi(IR_SEND_PIN);
IRMitsubishi136   acMitsubishi136(IR_SEND_PIN);
IRMitsubishi112   acMitsubishi112(IR_SEND_PIN);
IRToshibaAC acToshiba(IR_SEND_PIN);


IRsend irsend(IR_SEND_PIN);
IRrecv irrecv(IR_RECV_PIN, 1024, 50, true);
decode_results results;


WebServer ws(80);
WiFiManager wifiManager;
SemaphoreHandle_t xSemaphore;

//TEMPERATURA
unsigned long serverStartTime = 0; // Armazena o momento em que o servidor foi iniciado
const unsigned long serverTimeout = 60000; // Tempo máximo que o servidor ficará ativo (1 minuto em milissegundos)
bool accessed = false; // Indica se o acesso ao endpoint ocorreu

#define VERSION "V.3.05"

MemoryManagment memory;
ServerHandle sh;
PayloadHandle ph;
WifiHandle wh;

#define RESET_DURATION  6
#define JSON_DOC_SIZE 1024
#define EEPROM_SIZE 2048

String version = "itw601w-v305";
int dashIndex = version.indexOf('-');
String versionStr  = version.substring(dashIndex + 2);;

String ssidTest = "IoTEw_2.4G";

String mqttHost     = "mqtt.iotew.com.br";
String mqttPort     = "1884";
String mqttUser     = "iotew";
String mqttPassword = "Iotew123";
String siteUrl = "https://iotew.com.br/updates/";
unsigned long interval        =   120000;

const char *ap_ssid = "ESP32-AP";
const char *ap_password = "12345678";

IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

bool setServer    = false;

String MacAddressString = "";

int addrReconnect ;
int addrSSID ;
int addrPass ;

const int resetButton    =  0;
volatile byte countPress =  0;
bool serverInitialized = false ;
bool isServerCreated = false ;

bool isCreatedWifi = false;


String receivedSSID, receivedPassword, customerName;
bool smartConfigDone = false;
bool espIDFStackStopped = false;
smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
const String delimiter = "{=$?}";

bool setReconnect = false;
int contReset = 0;
bool updateNow = false;
// ============================ INTERFACE UNIVERSAL ============================
typedef void (*BeginFn)(void*);
typedef void (*SendFn)(void*);
typedef void (*PowerFn)(void*);
typedef void (*SetTempFn)(void*, int);
typedef int  (*GetTempFn)(void*);
typedef void (*ModeFn)(void*);

struct ACInterface {
  void* obj;
  BeginFn begin;
  SendFn send;
  PowerFn on;
  PowerFn off;
  SetTempFn setTemp;
  GetTempFn getTemp;
  PowerFn tempUp;
  PowerFn tempDown;
  ModeFn setCool;
  ModeFn setDry;
  ModeFn setFan;
  const char* name;
  decode_type_t proto;
};

// ============================ MACRO PADRÃO ============================
#define DEF_DRIVER(CLASS, INSTANCE, PROTO, NAME, COOL, DRY, FAN) \
  static void begin_##INSTANCE(void* o){ ((CLASS*)o)->begin(); } \
  static void send_##INSTANCE(void* o){ ((CLASS*)o)->send(); } \
  static void on_##INSTANCE(void* o){ ((CLASS*)o)->on(); } \
  static void off_##INSTANCE(void* o){ ((CLASS*)o)->off(); } \
  static void settemp_##INSTANCE(void* o, int t){ ((CLASS*)o)->setTemp(t); } \
  static int  gettemp_##INSTANCE(void* o){ return ((CLASS*)o)->getTemp(); } \
  static void tempup_##INSTANCE(void* o){ ((CLASS*)o)->setTemp(((CLASS*)o)->getTemp()+1); } \
  static void tempdown_##INSTANCE(void* o){ ((CLASS*)o)->setTemp(((CLASS*)o)->getTemp()-1); } \
  static void setcool_##INSTANCE(void* o){ ((CLASS*)o)->setMode(COOL); } \
  static void setdry_##INSTANCE(void* o){ ((CLASS*)o)->setMode(DRY); } \
  static void setfan_##INSTANCE(void* o){ ((CLASS*)o)->setMode(FAN); } \
  ACInterface drv_##INSTANCE = { \
                                 &INSTANCE, begin_##INSTANCE, send_##INSTANCE, on_##INSTANCE, off_##INSTANCE, \
                                 settemp_##INSTANCE, gettemp_##INSTANCE, tempup_##INSTANCE, tempdown_##INSTANCE, \
                                 setcool_##INSTANCE, setdry_##INSTANCE, setfan_##INSTANCE, NAME, PROTO \
                               };


#define DEF_DRIVER_MIDEA(CLASS, INSTANCE, PROTO, NAME, COOL, DRY, FAN) \
  static void begin_##INSTANCE(void* o){ ((CLASS*)o)->begin(); } \
  static void send_##INSTANCE(void* o){ ((CLASS*)o)->send(); } \
  static void on_##INSTANCE(void* o){ ((CLASS*)o)->setPower(true); } \
  static void off_##INSTANCE(void* o){ ((CLASS*)o)->setPower(false); } \
  static void settemp_##INSTANCE(void* o, int t){ ((CLASS*)o)->setTemp(t, true); } \
  static int  gettemp_##INSTANCE(void* o){ return ((CLASS*)o)->getTemp(true); } \
  static void tempup_##INSTANCE(void* o){ \
    int t = ((CLASS*)o)->getTemp(true); \
    ((CLASS*)o)->setTemp(t + 1, true); \
  } \
  static void tempdown_##INSTANCE(void* o){ \
    int t = ((CLASS*)o)->getTemp(true); \
    ((CLASS*)o)->setTemp(t - 1, true); \
  } \
  static void setcool_##INSTANCE(void* o){ ((CLASS*)o)->setMode(COOL); } \
  static void setdry_##INSTANCE(void* o){ ((CLASS*)o)->setMode(DRY); } \
  static void setfan_##INSTANCE(void* o){ ((CLASS*)o)->setMode(FAN); } \
  ACInterface drv_##INSTANCE = { \
                                 &INSTANCE, begin_##INSTANCE, send_##INSTANCE, on_##INSTANCE, off_##INSTANCE, \
                                 settemp_##INSTANCE, gettemp_##INSTANCE, tempup_##INSTANCE, tempdown_##INSTANCE, \
                                 setcool_##INSTANCE, setdry_##INSTANCE, setfan_##INSTANCE, NAME, PROTO \
                               };


#define DEF_DRIVER_GREE(CLASS, INSTANCE, PROTO, NAME, COOL, DRY, FAN) \
  static void begin_##INSTANCE(void* o){ ((CLASS*)o)->begin(); } \
  static void send_##INSTANCE(void* o){ ((CLASS*)o)->send(); } \
  static void on_##INSTANCE(void* o){ ((CLASS*)o)->setPower(true); } \
  static void off_##INSTANCE(void* o){ ((CLASS*)o)->setPower(false); } \
  static void settemp_##INSTANCE(void* o, int t){ ((CLASS*)o)->setTemp(t, false); } \
  static int  gettemp_##INSTANCE(void* o){ return ((CLASS*)o)->getTemp(); } \
  static void tempup_##INSTANCE(void* o){ \
    int t = ((CLASS*)o)->getTemp(); \
    ((CLASS*)o)->setTemp(t + 1, false); \
  } \
  static void tempdown_##INSTANCE(void* o){ \
    int t = ((CLASS*)o)->getTemp(); \
    ((CLASS*)o)->setTemp(t - 1, false); \
  } \
  static void setcool_##INSTANCE(void* o){ ((CLASS*)o)->setMode(COOL); } \
  static void setdry_##INSTANCE(void* o){ ((CLASS*)o)->setMode(DRY); } \
  static void setfan_##INSTANCE(void* o){ ((CLASS*)o)->setMode(FAN); } \
  ACInterface drv_##INSTANCE = { \
                                 &INSTANCE, begin_##INSTANCE, send_##INSTANCE, \
                                 on_##INSTANCE, off_##INSTANCE, \
                                 settemp_##INSTANCE, gettemp_##INSTANCE, \
                                 tempup_##INSTANCE, tempdown_##INSTANCE, \
                                 setcool_##INSTANCE, setdry_##INSTANCE, setfan_##INSTANCE, \
                                 NAME, PROTO \
                               };

#define DEF_DRIVER_WITH_SETPWR(CLASS, INSTANCE, PROTO, NAME, MODE_COOL, MODE_DRY, MODE_FAN) \
  static void begin_##INSTANCE(void* o){ ((CLASS*)o)->begin(); } \
  static void send_##INSTANCE(void* o){ ((CLASS*)o)->send(); } \
  static void on_##INSTANCE(void* o){ ((CLASS*)o)->setPower(true); } \
  static void off_##INSTANCE(void* o){ ((CLASS*)o)->setPower(false); } \
  static void settemp_##INSTANCE(void* o, int t){ ((CLASS*)o)->setTemp(t); } \
  static int gettemp_##INSTANCE(void* o){ return ((CLASS*)o)->getTemp(); } \
  static void tempup_##INSTANCE(void* o){ \
    int t = ((CLASS*)o)->getTemp(); \
    ((CLASS*)o)->setTemp(t + 1); \
  } \
  static void tempdown_##INSTANCE(void* o){ \
    int t = ((CLASS*)o)->getTemp(); \
    ((CLASS*)o)->setTemp(t - 1); \
  } \
  static void modecool_##INSTANCE(void* o){ ((CLASS*)o)->setMode(MODE_COOL); } \
  static void modedry_##INSTANCE(void* o){ ((CLASS*)o)->setMode(MODE_DRY); } \
  static void modefan_##INSTANCE(void* o){ ((CLASS*)o)->setMode(MODE_FAN); } \
  ACInterface drv_##INSTANCE = { \
                                 &INSTANCE, \
                                 begin_##INSTANCE, send_##INSTANCE, \
                                 on_##INSTANCE, off_##INSTANCE, \
                                 settemp_##INSTANCE, gettemp_##INSTANCE, \
                                 tempup_##INSTANCE, tempdown_##INSTANCE, \
                                 modecool_##INSTANCE, modedry_##INSTANCE, modefan_##INSTANCE, \
                                 NAME, PROTO \
                               };

#define DEF_DRIVER_PANA32(CLASS, INSTANCE, PROTO, NAME, MODE_COOL, MODE_DRY, MODE_FAN) \
  static void begin_##INSTANCE(void* o){ ((CLASS*)o)->begin(); } \
  static void send_##INSTANCE(void* o){ ((CLASS*)o)->send(); } \
  static void on_##INSTANCE(void* o){ ((CLASS*)o)->setPowerToggle(true); } \
  static void off_##INSTANCE(void* o){ ((CLASS*)o)->setPowerToggle(true); } \
  static void settemp_##INSTANCE(void* o, int t){ ((CLASS*)o)->setTemp(t); } \
  static int gettemp_##INSTANCE(void* o){ return ((CLASS*)o)->getTemp(); } \
  static void tempup_##INSTANCE(void* o){ \
    int t = ((CLASS*)o)->getTemp(); \
    ((CLASS*)o)->setTemp(t + 1); \
  } \
  static void tempdown_##INSTANCE(void* o){ \
    int t = ((CLASS*)o)->getTemp(); \
    ((CLASS*)o)->setTemp(t - 1); \
  } \
  static void modecool_##INSTANCE(void* o){ ((CLASS*)o)->setMode(MODE_COOL); } \
  static void modedry_##INSTANCE(void* o){ ((CLASS*)o)->setMode(MODE_DRY); } \
  static void modefan_##INSTANCE(void* o){ ((CLASS*)o)->setMode(MODE_FAN); } \
  ACInterface drv_##INSTANCE = { \
                                 &INSTANCE, \
                                 begin_##INSTANCE, send_##INSTANCE, \
                                 on_##INSTANCE, off_##INSTANCE, \
                                 settemp_##INSTANCE, gettemp_##INSTANCE, \
                                 tempup_##INSTANCE, tempdown_##INSTANCE, \
                                 modecool_##INSTANCE, modedry_##INSTANCE, modefan_##INSTANCE, \
                                 NAME, PROTO \
                               };

#define DEF_DRIVER_FUJITSU(CLASS, INSTANCE, PROTO, NAME, MODE_COOL, MODE_DRY, MODE_FAN) \
  static void begin_##INSTANCE(void* o){ ((CLASS*)o)->begin(); } \
  static void send_##INSTANCE(void* o){ ((CLASS*)o)->send(); } \
  static void on_##INSTANCE(void* o){ ((CLASS*)o)->setCmd(kFujitsuAcCmdTurnOn); } \
  static void off_##INSTANCE(void* o){ ((CLASS*)o)->setCmd(kFujitsuAcCmdTurnOff); } \
  static void settemp_##INSTANCE(void* o, int t){ ((CLASS*)o)->setTemp((float)t); } \
  static int gettemp_##INSTANCE(void* o){ return (int)((CLASS*)o)->getTemp(); } \
  static void tempup_##INSTANCE(void* o){ ((CLASS*)o)->setTemp(((CLASS*)o)->getTemp() + 1.0f); } \
  static void tempdown_##INSTANCE(void* o){ ((CLASS*)o)->setTemp(((CLASS*)o)->getTemp() - 1.0f); } \
  static void modecool_##INSTANCE(void* o){ ((CLASS*)o)->setMode(MODE_COOL); } \
  static void modedry_##INSTANCE(void* o){ ((CLASS*)o)->setMode(MODE_DRY); } \
  static void modefan_##INSTANCE(void* o){ ((CLASS*)o)->setMode(MODE_FAN); } \
  ACInterface drv_##INSTANCE = { \
                                 &INSTANCE, begin_##INSTANCE, send_##INSTANCE, \
                                 on_##INSTANCE, off_##INSTANCE, \
                                 settemp_##INSTANCE, gettemp_##INSTANCE, \
                                 tempup_##INSTANCE, tempdown_##INSTANCE, \
                                 modecool_##INSTANCE, modedry_##INSTANCE, modefan_##INSTANCE, \
                                 NAME, PROTO \
                               };

#define DEF_DRIVER_KELON(CLASS, INSTANCE, PROTO, NAME, MODE_COOL, MODE_DRY, MODE_FAN) \
  static void begin_##INSTANCE(void* o){ ((CLASS*)o)->begin(); } \
  static void send_##INSTANCE(void* o){ ((CLASS*)o)->send(); } \
  static void on_##INSTANCE(void* o){ ((CLASS*)o)->setTogglePower(true); } \
  static void off_##INSTANCE(void* o){ ((CLASS*)o)->setTogglePower(true); } \
  static void settemp_##INSTANCE(void* o, int t){ ((CLASS*)o)->setTemp(t); } \
  static int gettemp_##INSTANCE(void* o){ return ((CLASS*)o)->getTemp(); } \
  static void tempup_##INSTANCE(void* o){ ((CLASS*)o)->setTemp(((CLASS*)o)->getTemp() + 1); } \
  static void tempdown_##INSTANCE(void* o){ ((CLASS*)o)->setTemp(((CLASS*)o)->getTemp() - 1); } \
  static void modecool_##INSTANCE(void* o){ ((CLASS*)o)->setMode(MODE_COOL); } \
  static void modedry_##INSTANCE(void* o){ ((CLASS*)o)->setMode(MODE_DRY); } \
  static void modefan_##INSTANCE(void* o){ ((CLASS*)o)->setMode(MODE_FAN); } \
  ACInterface drv_##INSTANCE = { \
                                 &INSTANCE, \
                                 begin_##INSTANCE, \
                                 send_##INSTANCE, \
                                 on_##INSTANCE, \
                                 off_##INSTANCE, \
                                 settemp_##INSTANCE, \
                                 gettemp_##INSTANCE, \
                                 tempup_##INSTANCE, \
                                 tempdown_##INSTANCE, \
                                 modecool_##INSTANCE, \
                                 modedry_##INSTANCE, \
                                 modefan_##INSTANCE, \
                                 NAME, \
                                 PROTO \
                               };


// ============================ DRIVERS ============================

void stopEspIDFStack() {
  // Primeiro pare o smartconfig
  esp_smartconfig_stop();

  // Depois desconecte e desative o WiFi
  esp_wifi_disconnect();
  esp_wifi_stop();

  // Remova os handlers de evento
  esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler);
  esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler);
  esp_event_handler_unregister(SC_EVENT, SC_EVENT_GOT_SSID_PSWD, &event_handler);

  // Finalmente desinicialize
  esp_wifi_deinit();

  // Limpe a rede
  esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  if (netif) {
    esp_netif_destroy(netif);
  }
  esp_netif_deinit();

  // Adicione um delay maior para garantir a conclusão
  vTaskDelay(pdMS_TO_TICKS(500));

  espIDFStackStopped = true;
  Serial.println("ESP-IDF stack finalizada");
  vTaskDelay(pdMS_TO_TICKS(500));

}

void connectWithArduinoWiFi() {
  // Aguarde um pouco antes de iniciar a nova conexão
  Serial.println("\nCadastrando Servidor...");

  delay(1000);
  std::string mqttHostStr = std::string(mqttHost.c_str());
  std::string mqttPortStr = std::string(mqttPort.c_str());
  std::string mqttUserStr = std::string(mqttUser.c_str());
  std::string mqttPasswordStr = std::string(mqttPassword.c_str());

  sh.setMqttHost(mqttHostStr);
  sh.setMqttPort(mqttPortStr);
  sh.setMqttUser(mqttUserStr);
  sh.setMqttPassword(mqttPasswordStr);
  Serial.println("\nConectando ao WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(receivedSSID.c_str(), receivedPassword.c_str());

  // Adicione um timeout mais claro
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nConexão falhou \nResetando...\n");
    ESP.restart();
  }
  else {
    Serial.println("\nConectado com sucesso!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    buildTopic();

  }
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
    smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *) event_data;

    String SSID        = String((char*)evt->ssid);
    String passAndName = String((char*)evt->password);

    Serial.println("\n=== SmartConfig RECEBIDO ===");
    Serial.printf(" SSID: %s\n", SSID.c_str());
    Serial.printf(" PASS AND ID: %s\n", passAndName.c_str());

    int pos = passAndName.indexOf(delimiter);

    if (pos >= 8) {
      receivedSSID = SSID;
      receivedPassword = passAndName.substring(0, pos);
      customerName = passAndName.substring(pos + delimiter.length());

      Serial.println("\nSSID: "      + receivedSSID);
      Serial.println("PASSWORD: "    + receivedPassword);
      Serial.println("CUSTOMER Name: " + customerName);

      smartConfigDone = true;

      /* addrReconnect = memory.getAddrReconnect();
        EEPROM.write(addrReconnect, true);
        EEPROM.commit();
        Serial.print("this is the reconnect: ");
        Serial.println(EEPROM.read(addrReconnect));*/
      isCreatedWifi = true;
      // setReconnect = true;
      serverInitialized = true; // Marca como inicializado
      EEPROM.write(200, serverInitialized);
      EEPROM.commit();
      vTaskDelay(pdMS_TO_TICKS(200));
      /* memory.saveMemoryStr(memory.getAddrPassword(), std::string(receivedPassword.c_str()));
        EEPROM.commit();
        vTaskDelay(pdMS_TO_TICKS(200));
        memory.saveMemoryStr(memory.getAddrSSID(), std::string (receivedSSID.c_str()));*/
      //stopEspIDFStack();
      vTaskDelay(pdMS_TO_TICKS(200));
      Serial.println("Saindo static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data");

    } else {
      Serial.println("\nErro: 'password+Id' com tamanho inválido \nReiniciando...\n");
      ESP.restart();
    }
  }
}

int firstReset;
unsigned long previousMillisReset = 0;  // Armazena o último tempo em que a variável foi resetada
const long intervalReset = 30000;       // Intervalo de 30 segundos (30.000 milissegundos)
unsigned long previousMillis = 0;  // Armazena o último tempo em que a variável foi resetada


// Samsung
DEF_DRIVER(IRSamsungAc, acSamsung, SAMSUNG_AC,
           "Samsung", kSamsungAcCool, kSamsungAcDry, kSamsungAcFan)

// Carrier 64-bit
DEF_DRIVER(IRCarrierAc64, acCarrier, CARRIER_AC64,
           "Carrier", kCarrierAc64Cool, kCarrierAc64Heat, kCarrierAc64Fan)

// Criando driver
DEF_DRIVER_MIDEA(IRMideaAC, acMidea, MIDEA, "Midea",
                 kMideaACCool, kMideaACDry, kMideaACFan)

DEF_DRIVER_GREE(IRGreeAC, acGree, GREE, "Gree",
                kGreeCool, kGreeDry, kGreeFan)

DEF_DRIVER(IRHitachiAc1,   acHitachi1,   HITACHI_AC1,   "Hitachi1",
           kHitachiAc1Cool,   kHitachiAc1Dry,   kHitachiAc1Fan)

DEF_DRIVER(IRHitachiAc264, acHitachi264, HITACHI_AC264, "Hitachi264",
           kHitachiAc264Cool, kHitachiAc264Dry, kHitachiAc264Fan)

DEF_DRIVER(IRHitachiAc344, acHitachi344, HITACHI_AC344, "Hitachi344",
           kHitachiAc344Cool, kHitachiAc344Dry, kHitachiAc344Fan)

DEF_DRIVER(IRHitachiAc424, acHitachi424, HITACHI_AC424, "Hitachi424",
           kHitachiAc424Cool, kHitachiAc424Dry, kHitachiAc424Fan)

DEF_DRIVER(IRHitachiAc296, acHitachi296, HITACHI_AC296, "Hitachi296",
           kHitachiAc296Cool, kHitachiAc296DryCool, kHitachiAc296FanAuto)

DEF_DRIVER_WITH_SETPWR (IRCoolixAC, acCoolix, COOLIX, "Coolix",
                        kCoolixCool, kCoolixDry, kCoolixFan)

DEF_DRIVER(IRPanasonicAc, acPanasonic, PANASONIC_AC, "Panasonic",
           kPanasonicAcCool, kPanasonicAcDry, kPanasonicAcFan)

DEF_DRIVER_PANA32(IRPanasonicAc32, acPanasonic32, PANASONIC_AC32, "Panasonic32",
                  kPanasonicAc32Cool, kPanasonicAc32Dry, kPanasonicAc32Fan)

DEF_DRIVER_FUJITSU(IRFujitsuAC, acFujitsu, FUJITSU_AC, "Fujitsu",
                   kFujitsuAcModeCool, kFujitsuAcModeDry, kFujitsuAcModeFan)

DEF_DRIVER_WITH_SETPWR(IRBosch144AC, acBosch, BOSCH144, "Bosch144",
                       kBosch144Cool, kBosch144Dry, kBosch144Fan)

DEF_DRIVER_KELON(IRKelonAc, acKelon, KELON, "Kelon",
                 kKelonModeCool, kKelonModeDry, kKelonModeFan)

DEF_DRIVER(IRMitsubishiAC, acMitsubishi, MITSUBISHI_AC, "Mitsubishi",
           kMitsubishiAcCool, kMitsubishiAcDry, kMitsubishiAcFan)

DEF_DRIVER(IRMitsubishi136, acMitsubishi136, MITSUBISHI136, "Mitsubishi136",
           kMitsubishi136Cool, kMitsubishi136Dry, kMitsubishi136Fan)

DEF_DRIVER(IRMitsubishi112, acMitsubishi112, MITSUBISHI112, "Mitsubishi112",
           kMitsubishi112Cool, kMitsubishi112Dry, kMitsubishi112Auto)

DEF_DRIVER(IRToshibaAC, acToshiba, TOSHIBA_AC, "Toshiba",
           kToshibaAcCool, kToshibaAcDry, kToshibaAcFan)


// Lista
ACInterface* allDrivers[] = {
  &drv_acSamsung,
  &drv_acCarrier,
  &drv_acMidea,
  &drv_acGree,
  &drv_acHitachi1,
  &drv_acHitachi264,
  &drv_acHitachi344,
  &drv_acHitachi424,
  &drv_acHitachi296,
  &drv_acCoolix,
  &drv_acPanasonic,
  &drv_acPanasonic32,
  &drv_acFujitsu,
  &drv_acBosch,
  &drv_acKelon,
  &drv_acMitsubishi,
  &drv_acMitsubishi136,
  &drv_acMitsubishi112,
  &drv_acToshiba
};

const int DRIVER_COUNT = 19;
ACInterface* activeAC = nullptr;

// ============================ FUNÇÕES ============================
ACInterface* findDriverByProto(decode_type_t proto) {
  for (int i = 0; i < DRIVER_COUNT; i++) {
    if (allDrivers[i]->proto == proto) return allDrivers[i];
  }
  return nullptr;
}

// Auto-detect
void autoDetectAC() {
  Serial.println("Aponte o controle e pressione um botão...");
  while (!irrecv.decode(&results)) delay(10);

  Serial.print("Protocolo detectado: ");
  Serial.println(resultToHumanReadableBasic(&results));

  ACInterface* drv = findDriverByProto(results.decode_type);
  if (drv) {
    activeAC = drv;
    drv->begin(drv->obj);
    Serial.print("Driver configurado: ");
    Serial.println(drv->name);
  } else {
    Serial.println("Driver não encontrado.");
  }

  irrecv.resume();
}

// Menus
void printMainMenu() {
  Serial.println("\n=== AC UNIVERSAL 2.0 ===");

  // Mostra o status atual
  Serial.print("Status: ");
  if (activeAC) Serial.println(activeAC->name);
  else Serial.println("Nenhum AC configurado");

  Serial.print("Termostato: ");
  Serial.println(thermostatEnabled ? "LIGADO [ON]" : "DESLIGADO [OFF]");

  Serial.print("Meta Temp:  ");
  Serial.print(targetTemp);
  Serial.println("C");

  Serial.println("\n=== AC UNIVERSAL ===");
  Serial.println("A   - Auto detectar (Aponte o controle)");
  Serial.println("M   - Selecionar driver manualmente");
  Serial.println("C   - Controle manual (Botões virtuais)");
  Serial.println("T   - Ligar/Desligar Termostato");
  Serial.println("Sxx - Mudar Meta Temp (ex: S23 para 23 graus)");
  Serial.println("L   - Listar drivers");
}

void printDrivers() {
  Serial.println("\nDrivers disponíveis:");
  for (int i = 0; i < DRIVER_COUNT; i++) {
    Serial.print(i + 1);
    Serial.print(" - ");
    Serial.println(allDrivers[i]->name);
  }
}

void printControlMenu() {
  Serial.print("\nControlando: ");
  Serial.println(activeAC->name);
  Serial.println("L - Ligar");
  Serial.println("D - Desligar");
  Serial.println("+ - Temp +1");
  Serial.println("- - Temp -1");
  Serial.println("SXX - Set temp (ex S23)");
  Serial.println("C - Cool");
  Serial.println("R - Dry");
  Serial.println("F - Fan");
  Serial.println("B - Enviar");
  Serial.println("V - Voltar");
}


struct Config {
  int protocolID; // ID numérico do protocolo (ex: SAMSUNG_AC)
  int targetTemp;
};



void setLed(bool r, bool g, bool b) {
  digitalWrite(PIN_LED_R, r);
  digitalWrite(PIN_LED_G, g);
  digitalWrite(PIN_LED_B, b);
}

// Função para Feedback (Piscar)
void blinkLed(int color, int times) {
  for (int i = 0; i < times; i++) {
    if (color == 1) setLed(0, 1, 0); // Verde
    if (color == 2) setLed(1, 0, 0); // Vermelho
    delay(200);
    setLed(0, 0, 0);
    delay(200);
  }
}

void saveConfig() {
  if (activeAC == nullptr) return;

  // Feedback: AZUL (Processo de gravação)
  setLed(0, 0, 1);

  Config cfg;
  cfg.protocolID = (int)activeAC->proto;
  cfg.targetTemp = targetTemp;

  EEPROM.put(0, cfg);
  EEPROM.commit();

  delay(500); // Simula tempo de processo
  setLed(0, 0, 0); // Apaga Azul

  // Feedback: VERDE Intermitente (Gravou)
  blinkLed(1, 3);
  Serial.println("Configurações salvas na EEPROM!");
}

void loadConfig() {
  Config cfg;
  EEPROM.get(0, cfg);

  // Tenta encontrar o driver salvo
  ACInterface* savedDriver = findDriverByProto((decode_type_t)cfg.protocolID);

  if (savedDriver != nullptr) {
    activeAC = savedDriver;
    activeAC->begin(activeAC->obj);
    targetTemp = cfg.targetTemp;

    Serial.print("Configuração carregada. AC: ");
    Serial.println(activeAC->name);
    Serial.print("Meta Temp: ");
    Serial.println(targetTemp);
  } else {
    Serial.println("Nenhuma configuração válida encontrada.");
  }
}




void sendActive() {
  if (!activeAC) return;
  activeAC->send(activeAC->obj);
}



// ============================ LOOP ============================
String buffer = "";
int state = 0; // 0 main, 1 control, 2 manual
int contInfo = 0;
