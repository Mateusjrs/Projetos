
#include <Ticker.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <EEPROM.h>
#include <DS3232RTC.h>      // https://github.com/JChristensen/DS3232RTC
#include <Streaming.h>      // https://github.com/janelia-arduino/Streaming
#include <TimeLib.h>
#include "Multiplexador.h"
#include <ArduinoJson.h>
#include <memoryManagment.h>
#include <ServerHandle.h>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip> //std::fixed and std::precision
#include <vector>
#include <time.h>
#include <WiFi.h>
#include <WebServer.h>
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <pgmspace.h>
#include "bitmap.h"
#include <PayloadHandle.h>
#include <WiFiManager.h>
#include <WifiHandle.h>
#include "sdkconfig.h"
#include "esp_idf_version.h"
#include <Adafruit_NeoPixel.h>
#include <HTTPClient.h>
#include <Adafruit_INA219.h>
#include "esp_smartconfig.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include <Preferences.h>
Preferences prefs;


String ssid ;
String password;


int ConnectCont = 0;

bool setReconnect = false;
int contReset = 0;
bool updateNow = false;

String receivedSSID, receivedPassword, customerName;
bool smartConfigDone = false;
bool espIDFStackStopped = false;
smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
const String delimiter = "{=$?}";

#define ledRgb 16

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, ledRgb, NEO_GRB + NEO_KHZ800);

#if CONFIG_ESP_WIFI_REMOTE_ENABLED
#error "SmartConfig is only supported in SoCs with native Wi-Fi support"
#endif


unsigned long serverStartTime = 0; // Armazena o momento em que o servidor foi iniciado
const unsigned long serverTimeout = 60000; // Tempo máximo que o servidor ficará ativo (1 minuto em milissegundos)
bool accessed = false; // Indica se o acesso ao endpoint ocorreu

TFT_eSPI tft = TFT_eSPI();


#define VERSION "V.3.06"
int WIDTH_ITEM_MENU  = 210;
const int HEIGHT_ITEM_MENU = 35;

uint16_t* coolerCopy;
uint16_t* heaterCopy;
uint16_t* feederSmallCopy;
uint16_t* lampSmallCopy;
uint16_t* wifiCopy;
uint16_t* serviceCopy;
uint16_t* errorCopy;

int horaAtual;
int minAtual;

bool priorityMqtt = true;
uint16_t grayColor = (16 << 11) | (32 << 5) | 16;


TaskHandle_t SwicthButtonHandle;
TaskHandle_t SwicthMenuHandle;
TaskHandle_t SwicthPayloadHandle;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

MultiplexadorHandle mut;
DS3232RTC myRTC;
WebServer ws(80);
WiFiManager wifiManager;

MemoryManagment memory;
ServerHandle sh;
PayloadHandle ph;
WifiHandle wh;
Ticker periodicTemp;
Ticker periodicServer;
Ticker readTemp;
Ticker readClock;
Ticker Preprogram;
Ticker errorProgram;
Ticker timeProgram;
Ticker displayCheck;

bool feederFooter = false;
bool lightFooter = false;
bool heaterFooter = false;
bool coolerFooter = false;

bool displayPriority = false;
/* ----HARDWARE MAPPING---- */
//**********************TESTE PROTOBOARD IOTEW ********************************



#define feeder  33
#define switchFeeder  25
#define cooler  12
#define heater  13
#define light1  14
#define light3  27
#define light2  26
#define oneWireBus 16 //D2
#define buttonUp 32
#define buttonDown 35
#define buttonGo 34
#define buttonFeeder  30

#define debouceTime 300  // Tempo de debounce em milissegundos

/* ---- CONFIG TICKER---- */
#define EEPROM_SIZE 2048
#define RESET_DURATION  6
#define JSON_DOC_SIZE 1024

#define I2C_SDA 21
#define I2C_SCL 22

OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

String version = "itw401w-v306";
int dashIndex = version.indexOf('-');
String versionStr  = version.substring(dashIndex + 2);;

String ssidTest = "IoTEw_2.4G";

String mqttHost     = "quili.iotew.com.br";
String mqttPort     = "1884";
String mqttUser     = "quili_iotew";
String mqttPassword = "123@Quili";
String siteUrl = "https://iotew.com.br/updates/";

/* ---- CONFIG PWM LIGHT ---- */
const int freq = 5000;
const int resolution = 8;
const int ledChannel = 0;
const int ledChanne2 = 2;
const int ledChanne3 = 3;


/* ----  PAYLOAD VARIABLES  ---- */
int intensityLight;
float temp;
unsigned long last;
unsigned long next;
unsigned long long timerRtc ;
unsigned long long signature;

/* ----  BUTTON CONTROLL   ---- */
volatile bool FlagBUp;             // External Interrupt -- Trigger Button
volatile bool FlagBDown;             // External Interruption -- Light Button
unsigned long lastButtonPressTime = 0;
const unsigned long pressInterval = 1000;
unsigned long lastButtonIntensity = 0;
const unsigned long stabilizationIntensity = 1000;
unsigned long interval        =   120000;


/* ----  MENU PARAMETROS  ---- */
bool auxFeeder ;
bool  indexTemp;
bool indexProgram;

/* ----  TEMP CONTROLL   ---- */
float idealTemp ;
int savetemp = 25;
float range = 1;

/* ----  MENU CONTROLL   ---- */
int menu = 0;
bool firstPression = false;
#define TEMPOENTERVOLTAR 2000// Tempo para o pressionar do botão computar como ENTER ou VOLTAR (em mS)
int plus = 0;

int timeFeeder = 10000;  // Tempo limite para verificar se esta travado
int lastExecutedMinute = -1; // Variável para armazenar o último minuto em que a função foi executada
int previousRoundedValue = 0;// Verifica se o valor arredeondado foi alterado
bool goIn;
int plusProgram = 1;
//int numProgram = 0;
//String nameProgram;

int intensityLight1, intensityLight2, intensityLight3;
float modificTempDTemp;

int tempProg;
int moduloError [6] = {0, 0, 0, 0, 0, 0}; // L1,L2,L3,Heater,Cooler,Feeder
int controllError [6] = {0, 0, 0, 0, 0, 0}; // L1,L2,L3,Heater,Cooler,Feeder
int valueError;
int preSelect = 0;
bool statusHeater = false;
bool statusCooler = false;

/* ----  PAYLOAD VARIABLES  ---- */
int Intensity_Light = 0 ;

/* ----  PAYLOAD ACTIVATION CONTROL   ---- */
int lastProgram ;    // Variable to store the last trigger state (ON--OFF)
int roundedIntensityValue;

unsigned long previousMillis = 0;
unsigned long previousMillisFeeder = 0;
const long intervalFeeder = 10000; // 10 segundos
bool feederActive = false;

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
int light;
float displayTemp;




bool modificTempM;
int Lamp1 = -1;
int Lamp2 = -1;
int Lamp3 = -1;


int mediaFeeder;
struct LampRoutine {
  int intensityL1;
  int intensityL2;
  int intensityL3;
  int hour;
  int minutes;
};

struct FeederRoutine {
  int spins;
  int hour;
  int minutes;
};
bool routineIlu = false;
bool routineFeeder = false;
FeederRoutine feederRoutines[5];  // Armazena até 5 rotinas
LampRoutine lampRoutines[5];  // Armazena até 5 rotinas
int currentRoutineIndexFeeder = 0;  // Índice da próxima rotina a ser armazenada
int currentRoutineIndexLamp = 0;  // Índice da próxima rotina a ser armazenada
int routineCountIlumi = 0;         // Contador de rotinas ativas
int routineCountFeeder = 0;         // Contador de rotinas ativas
bool customMqtt = false;
int lastValue[6] = {0, 0, 0, 0, 0, 0}; // Vetor para armazenar as últimas intensidades
int changeValue[6] = {0, 0, 0, 0, 0, 0}; // Vetor para armazenar as últimas intensidades

float tempMqtt;
#define MAX_FEEDER_ROUTINES 5  // Número máximo de rotinas que queremos armazenar
#define MAX_LAMP_ROUTINES 5  // Número máximo de rotinas de iluminação que queremos armazenar

int timeLamp1 = -1;
int timeLamp2 = -1;
int timeLamp3 = -1;

unsigned long lastMenuPress = 0;  // Variável para armazenar o tempo da última pressão de qualquer botão
bool noButtonPressed = false;       // Variável booleana que será true se nenhum botão for pressionado por mais de 1 minuto
const unsigned long timeout = 40000; // Tempo limite de 1 minuto (em milissegundos)
unsigned long long SignatureMqtt = 0;
unsigned long long currentRtc = 0;
bool signatureAdjustMqtt ;
bool timeAjustMqtt;
bool feederNowMqtt;
bool lampImmediateMqtt;
bool lampRoutineMqtt;
bool feederRoutineMqtt;
bool tempAdjustMqtt;
String timeLamp;
int timefeederSpins;
unsigned long long rtcMqtt = 0;
String timeFeederMqtt ;
//int timefeederSpins;
float rangeMqtt = 0;


unsigned long portalStartTime = 0;
const unsigned long portalTimeout = 60000; // 1 minuto em milissegundos

int rotation = 1;
bool isTempValid = false;
bool serverInitialized = false ;
bool isServerCreated = false ;

bool isCreatedWifi = false;

String feederMode = "DESATIVADO";
int feedingHour = 9;
int feedingMinute = 0;

String lastfeederOn = "DESATIVADO";
int lastfeedingHour = 9;
int lastfeedingMinute = 0;


String statusIlum = "DESATIVADO";
int illuminationEndHour = 10;
int illuminationEndMinute = 0;
int illuminationStartHour = 9;
int illuminationStartMinute = 0;

String laststatusIlum = "DESATIVADO";
int lastilluminationEndHour = 10;
int lastilluminationEndMinute = 0 ;
int lastilluminationStartHour = 9;
int lastilluminationStartMinute = 0;

// Instâncias dos sensores com endereços I2C diferentes
Adafruit_INA219 ina219_1(0x40);
Adafruit_INA219 ina219_2(0x41);
Adafruit_INA219 ina219_3(0x42);
Adafruit_INA219 ina219_5(0x44);

float current1 = 0.0;
float current2 = 0.0;
float current3 = 0.0;
float current4 = 0.0;
float current5 = 0.0;

int firstReset;
bool  modificTempP = 0;

String macCompare;
String MACADRESS;
bool sameWifi;
bool forceClockRedraw = false;

#define EEPROM_LAMP_ROUTINES    1200
#define EEPROM_FEEDER_ROUTINES  1400
//************************************************************************ FEEDER**************************************************************************
void stopEspIDFStack() {
  // 1. Pare apenas o smartconfig
  esp_smartconfig_stop();

  // 2. Remova os handlers de evento nativos
  esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler);
  esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler);
  esp_event_handler_unregister(SC_EVENT, SC_EVENT_GOT_SSID_PSWD, &event_handler);
  esp_event_handler_unregister(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler); // Garante a remoção do que foi registrado no setup

  // 3. Desconecte usando o Arduino, NÃO use esp_wifi_stop() ou esp_wifi_deinit()
  WiFi.disconnect(true);

  // Adicione um delay para garantir que as tarefas de rede do core se estabilizem
  vTaskDelay(pdMS_TO_TICKS(500));

  espIDFStackStopped = true;
  Serial.println("SmartConfig nativo finalizado (Stack WiFi mantida para o Arduino)");
}

/*
  void connectWithArduinoWiFi() {

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

  // SE veio do SmartConfig NÃO conecta de novo
  if (smartConfigDone) {
    Serial.println("WiFi já conectado pelo SmartConfig.");
    buildTopic();
    sh.connectToMQTTServer();
    return;
  }

  Serial.println("\nConectando ao WiFi...");

  WiFi.mode(WIFI_STA);
  vTaskDelay(pdMS_TO_TICKS(150));

  WiFi.begin(receivedSSID.c_str(), receivedPassword.c_str());

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
*/

volatile bool flagStartMQTT = false;
void connectWithArduinoWiFi() {

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

  if (smartConfigDone) {
    Serial.println("WiFi já está conectando pelo SmartConfig...");
    return;
  }

  Serial.println("\nConectando ao WiFi...");

  WiFi.mode(WIFI_STA);
  vTaskDelay(pdMS_TO_TICKS(150));

  WiFi.begin(receivedSSID.c_str(), receivedPassword.c_str());

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nConexão falhou \nResetando...\n");
    ESP.restart();
  }

  Serial.println("\nConectado com sucesso!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  buildTopic();
}
bool ackSent = false;
bool wifiConnected = false;

static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{

  // ==============================
  // WIFI CONECTOU
  // ==============================
  if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    wifiConnected = true;

    Serial.println("\nWiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    Serial.println("\nSinalizando para o loop iniciar o MQTT...");

    // ATENÇÃO: Nunca conecte o MQTT aqui dentro!
    // Apenas acione a flag para o loop fazer isso.
    flagStartMQTT = true;
  }


  // ==============================
  // ACK enviado ao celular
  // ==============================
  if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE)
  {
    ackSent = true;
    Serial.println("ACK enviado ao smartphone.");
  }


  // ==============================
  // FINALIZA SMARTCONFIG
  // ==============================
  if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE)
  {
    Serial.println("ACK enviado ao smartphone.");
    esp_smartconfig_stop();
  }


  // ==============================
  // RECEBEU SSID E SENHA
  // ==============================
  if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD)
  {
    smartconfig_event_got_ssid_pswd_t *evt =
      (smartconfig_event_got_ssid_pswd_t *)event_data;

    String SSID = String((char*)evt->ssid);
    String passAndName = String((char*)evt->password);

    Serial.println("\n=== SmartConfig RECEBIDO ===");
    Serial.printf("SSID: %s\n", SSID.c_str());
    Serial.printf("PASS AND ID: %s\n", passAndName.c_str());

    int pos = passAndName.indexOf(delimiter);

    if (pos < 8)
    {
      Serial.println("\nErro: formato inválido de password+Id");
      Serial.println("Reiniciando dispositivo...\n");

      ESP.restart();
      return;
    }

    // ==============================
    // EXTRAÇÃO DOS DADOS
    // ==============================

    receivedSSID     = SSID;
    receivedPassword = passAndName.substring(0, pos - 7);
    ssid             = receivedSSID;

    customerName = passAndName.substring(pos + delimiter.length());
    macCompare   = passAndName.substring(pos - 7, pos);

    macCompare.replace(":", "");

    Serial.println("\n=== Dados Extraídos ===");
    Serial.println("SSID: " + receivedSSID);
    Serial.println("PASSWORD: " + receivedPassword);
    Serial.println("CUSTOMER: " + customerName);
    Serial.println("MAC Recebido: " + macCompare);
    Serial.println("MAC Local: " + MACADRESS);

    // ==============================
    // VERIFICAÇÃO DO MAC
    // ==============================

    // ==============================
    // VERIFICAÇÃO DO MAC
    // ==============================

    if (MACADRESS == macCompare)
    {
      Serial.println("MACs conferem — prosseguindo...");

      sameWifi = true;
      smartConfigDone = true;
      isCreatedWifi = true;
      serverInitialized = true;

      prefs.putBool("serverInit", serverInitialized);

      Serial.println("SmartConfig Done");

      // ==========================================
      // A MÁGICA ACONTECE AQUI
      // ==========================================
      // 1. Forçamos a desconexão para limpar qualquer estado
      //    pendente e evitar o erro "cannot set config"
      WiFi.disconnect();
      vTaskDelay(pdMS_TO_TICKS(50)); // Pequeno fôlego para o driver de rede

      // 2. Mandamos conectar usando as credenciais que acabamos de extrair
      Serial.println("\nIniciando conexão WiFi com as credenciais do SmartConfig...");
      WiFi.begin(receivedSSID.c_str(), receivedPassword.c_str());
      // ==========================================
    }
    else
    {
      Serial.println("\nErro: MAC recebido não corresponde ao MAC do dispositivo");
      Serial.println("Reiniciando...\n");

      ESP.restart();
    }
  }
}



void handleFeederActivation(unsigned long feederTime) {

  if (digitalRead(feeder) == false) {
    Serial.println("Ativando Alimentador");
    digitalWrite(feeder, HIGH);
    feederFooter = true;
    if (firstPression == false) {
      drawFooter(coolerFooter, heaterFooter, feederFooter, lightFooter);
    }
    unsigned long startTime = millis();
    sendDataFeeder();
    vTaskDelay(pdMS_TO_TICKS(100));
    while (millis() - startTime < feederTime) {
      if (digitalRead(switchFeeder) == LOW) {
        digitalWrite(feeder, LOW);
        feederFooter = false;
        Serial.println("Desativando Alimentador if");
        if (firstPression == false) {
          drawFooter(coolerFooter, heaterFooter, feederFooter, lightFooter);
        }
        sendDataFeeder();
        vTaskDelay(pdMS_TO_TICKS(100));
        break;
        //return;
      }
      vTaskDelay(pdMS_TO_TICKS(100));
    }
    Serial.println("Desativando Alimentador");
    digitalWrite(feeder, LOW);
    feederFooter = false;

  }
  vTaskDelay(pdMS_TO_TICKS(100));
}

void activateFeeder() {
  handleFeederActivation(timeFeeder);
}

void activateFeederMqtt(int numFeeder) {
  for (int i = 0; i < numFeeder ; i++) {
    handleFeederActivation(timeFeeder);
    vTaskDelay(pdMS_TO_TICKS(300));
  }
  return;
}

void controllFood(int targetHour, int targetMinute, int numFeeder ) {
  time_t now = myRTC.get();  // Obtém o tempo atual do RTC
  tmElements_t tm;
  breakTime(now, tm);  // Decompõe o tempo em componentes
  if (tm.Hour == targetHour && tm.Minute == targetMinute ) {
    if (tm.Minute != lastExecutedMinute) {
      Serial << "Current Time: " << tm.Hour << ":" << tm.Minute << " - Time to feed!" << endl;
      Serial.println("Hora bateu");
      activateFeederMqtt(numFeeder);
      lastExecutedMinute = tm.Minute;
    }
  }
}

//************************************************************************ FIM FEEDER **************************************************************************
//************************************************************************ LIGHT **************************************************************************
int mapCorrection(int input) {
  // Aplica uma correção específica para entradas de 1 a 10
  switch (input) {
    case 1: return 10;   // Compensa para 10% (representado pelo valor 1)
    case 2: return 15;   // Compensa para 20% (representado pelo valor 2)
    case 3: return 25;   // Compensa para 30% (representado pelo valor 3)
    case 4: return 30;  // Compensa para 40% (representado pelo valor 4)
    case 5: return 50;  // Compensa para 50% (representado pelo valor 5)
    case 6: return 90;  // Compensa para 60% (representado pelo valor 6)
    case 7: return 120;  // Compensa para 70% (representado pelo valor 7)
    case 8: return 170;  // Compensa para 80% (representado pelo valor 8)
    case 9: return 200;  // Compensa para 90% (representado pelo valor 9)
    case 10: return 224; // Compensa para 100% (representado pelo valor 10)
    default: return 0;   // Valor de segurança para fora do intervalo esperado
  }

}

void updateLightIntensity(int light, int intensityLight) {
  int correctedIntensity = mapCorrection(intensityLight);

  ledcWrite(light, correctedIntensity);
}

int cont;
void activateLight() {
  cont = cont + 1;
  Serial.println(cont);
  displayPriority = true;
  intensityLight++;
  intensityLight1 = intensityLight2 = intensityLight3 = intensityLight;
  previousRoundedValue = -1;  // Reseta o valor ao sobrescrever manualmente
  lightFooter = true;
  priorityMqtt = false;

  if (intensityLight1 != intensityLight2 or  intensityLight2 != intensityLight3) {
    float media = (intensityLight1 + intensityLight2 + intensityLight3) / 3.0;
    int dezena = constrain(round(media / 10) * 10, 10, 100);
    intensityLight = dezena / 10;
    intensityLight1 = intensityLight2 = intensityLight3 = intensityLight;
    checkAll();
  }
  if (intensityLight < 1 || intensityLight > 10) {
    plus = 1;
    intensityLight = intensityLight1 = intensityLight2 = intensityLight3 = 1;
  }
  if (intensityLight == 11) {
    intensityLight = intensityLight1 = intensityLight2 = intensityLight3 = 1;
  }
  updateLightIntensity(light1, intensityLight1);
  updateLightIntensity(light2, intensityLight2);
  updateLightIntensity(light3, intensityLight3);
  vTaskDelay(pdMS_TO_TICKS(150));

}

void activateLightMqtt(int intensityLightMqtt1, int intensityLightMqtt2, int intensityLightMqtt3, int startHour, int startMinute) {
  time_t now = myRTC.get();  // Obtém o tempo atual do RTC
  tmElements_t tm;
  breakTime(now, tm);  // Decompõe o tempo em componentes
  int startMinutes = startHour * 60 + startMinute;
  int currentMinutes = tm.Hour * 60 + tm.Minute;

  if (startHour == tm.Hour && tm.Minute == startMinute) {

    if (horaAtual != startHour && horaAtual != startMinute) {
      horaAtual = startHour;
      minAtual = startMinute;
      priorityMqtt = true;
    }
    if (priorityMqtt) {
      intensityLight1 = intensityLightMqtt1 / 10;
      intensityLight2 = intensityLightMqtt2 / 10;
      intensityLight3 = intensityLightMqtt3 / 10;
      if (intensityLight1 <= 100 && intensityLight1 >= 0 && intensityLight2 <= 100 && intensityLight2 >= 0 && intensityLight3 <= 100 && intensityLight3 >= 0) {
        updateLightIntensity(light1, intensityLightMqtt1 / 10);
        updateLightIntensity(light2, intensityLightMqtt2 / 10);
        updateLightIntensity(light3, intensityLightMqtt3 / 10);
      }

      else if (currentMinutes < startMinutes )  {
        ledcWrite(ledChannel, 0);
        ledcWrite(ledChanne2, 0);
        ledcWrite(ledChanne3, 0);
      }

      if (intensityLight1 == 0 && intensityLight2 == 0 && intensityLight3 == 0 ) {
        lightFooter = false;
      }
      if (intensityLight1 > 0 || intensityLight2 > 0 || intensityLight3 > 0)  {
        lightFooter = true;
      }
    }
  }
  else {
  }
}

int hourNow;
int minuteNow;

void controllLed( int startHour, int startMinute, int endHour, int endMinute) {
  time_t now = myRTC.get();  // Obtém o tempo atual do RTC
  tmElements_t tm;
  breakTime(now, tm);  // Decompõe o tempo em componentes
  int startMinutes = startHour * 60 + startMinute;
  int currentMinutes = tm.Hour * 60 + tm.Minute;
  int endMinutes = endHour * 60 + endMinute;

  if (startHour == tm.Hour && tm.Minute == startMinute) {

    if (hourNow != startHour && minuteNow != startMinute) {
      hourNow = startHour;
      minuteNow = startMinute;
    }
    intensityLight1 = 10;
    intensityLight2 = 10;
    intensityLight3 = 10;
    updateLightIntensity(light1, 10);
    updateLightIntensity(light2, 10);
    updateLightIntensity(light3, 10);

    if (intensityLight1 == 0 && intensityLight2 == 0 && intensityLight3 == 0 ) {
      lightFooter = false;
    }
    if (intensityLight1 > 0 || intensityLight2 > 0 || intensityLight3 > 0)  {
      lightFooter = true;
    }
  }
  else if (endHour == tm.Hour && tm.Minute == endMinute) {

    if (hourNow != endHour && minuteNow != endMinute) {
      hourNow = endHour;
      minuteNow = endMinute;
    }
    intensityLight1 = 0;
    intensityLight2 = 0;
    intensityLight3 = 0;
    updateLightIntensity(light1, intensityLight1);
    updateLightIntensity(light2, intensityLight1);
    updateLightIntensity(light3, intensityLight1);

    if (intensityLight1 == 0 && intensityLight2 == 0 && intensityLight3 == 0 ) {
      lightFooter = false;
    }
    if (intensityLight1 > 0 || intensityLight2 > 0 || intensityLight3 > 0)  {
      lightFooter = true;
    }

  }

  else {
  }
}





/*
  void controllLed(int startHour, int startMinute, int endHour, int endMinute) {
    time_t now = myRTC.get();  // Obtém o tempo atual do RTC
    tmElements_t tm;
    breakTime(now, tm);  // Decompõe o tempo em componentes
    int startMinutes = startHour * 60 + startMinute;
    int endMinutes = endHour * 60 + endMinute;
    int currentMinutes = tm.Hour * 60 + tm.Minute;


    if (currentMinutes >= startMinutes && currentMinutes < endMinutes) {
      intensityLight1 = 10;
      intensityLight2 = 10;
      intensityLight3 = 10;
      updateLightIntensity(light1, intensityLight1);
      updateLightIntensity(light2, intensityLight2);
      updateLightIntensity(light3, intensityLight3);
      displayPriority = false;
      lightFooter = true;
      checkAll();
    }
    else if (currentMinutes >= endMinutes && displayPriority == false) {
      intensityLight1 = 0;
      intensityLight2 = 0;
      intensityLight3 = 0;
      updateLightIntensity(light1, 0);
      updateLightIntensity(light2, 0);
      updateLightIntensity(light3, 0);
      lightFooter = false;
      return;
    }
  }
*/

//************************************************************************ FIM LIGHT **************************************************************************
//************************************************************************ CONTROLL TEMP **************************************************************************

void activateHeater(bool statusHeater) {
  if (statusHeater == 1) {
    digitalWrite(heater, HIGH);
    heaterFooter = true;
  }
  if (statusHeater == 0) {
    digitalWrite(heater, LOW);
    heaterFooter = false;
  }
}

void activateCooler(bool statusCooler) {
  if (statusCooler == 1) {
    digitalWrite(cooler, HIGH);
    coolerFooter = true;
  }
  if (statusCooler == 0) {
    digitalWrite(cooler, LOW);
    coolerFooter = false;
  }
}


unsigned long lastTempChangeTime = 0;  // Marca o tempo da última mudança de temperatura
float lastTemp = -127.0;               // Armazena a última temperatura lida
bool controlActive = true;             // Indica se o controle de temperatura está ativo
const float maxTemp = 40.0;            // Temperatura máxima permitida
const float minTemp = 5.0;             // Temperatura mínima permitida
bool constTemp = false;                // Indica se a temperatura está constante

void controllTemp(float temp, float idealTemp, float range) {

  isTempValid = true;

  // Verifica se a temperatura está fora dos limites
  if (temp > maxTemp || temp < minTemp) {
    activateCooler(0);
    activateHeater(0);
    controlActive = false;  // Desativa o controle de temperatura
    return;
  }

  // Verifica se a temperatura está constante por 60 segundos
  if (millis() - lastTempChangeTime >= 720000 && controlActive && round(temp) == round(lastTemp)) {
    //Serial.println("Temperatura constante por 60 segundos.");
    constTemp = true;
    activateHeater(0);
    if (round(temp) > round(lastTemp)) {
      //Serial.println("Atualizando temperatura superior à constante.");
      lastTemp = temp;  // Atualiza apenas se for superior
      lastTempChangeTime = millis();
    } else {
      Serial.println("Temperatura não superior à constante. Mantendo.");
      return;  // Não atualiza se não for superior
    }
  } else {
    constTemp = false;
  }

  // Controle de temperatura
  if (controlActive && isTempValid) {
    if (temp <= (savetemp + range) && temp >= (savetemp - range)) {
      activateHeater(0);
      activateCooler(0);
      Serial.println("Ideal");
    } else if (temp < savetemp - range) {
      activateHeater(1);
      activateCooler(0);
      Serial.println("Aquecendo");
    } else if (temp > savetemp + range) {
      activateHeater(0);
      activateCooler(1);
      Serial.println("Resfriando");
    }
    Serial.println();
    checkAll();
  } else {
    //Serial.println("Controle de temperatura inválido.");
    activateCooler(0);
    activateHeater(0);
  }
}

void controllTempWrapper() {
  controllTemp(temp, idealTemp, range);
}


void onSensorsIDTemp() {
  sensors.requestTemperatures();
  temp = sensors.getTempCByIndex(0);
  isTempValid = true;

  // Valida a temperatura obtida
  if (temp == -127) {
    temp = -30.0;  // Define um valor padrão em caso de erro
    isTempValid = false;  // Temperatura inválida
  }

  // Verifica se o range é válido
  if (range == 0 || range > 6) {
    isTempValid = false;
  }

  // Atualiza o tempo da última mudança e temperatura apenas se necessário
  if (round(temp) != round(lastTemp)) {
    lastTempChangeTime = millis();  // Atualiza o tempo da última mudança
    lastTemp = temp;                // Atualiza a última temperatura
    controlActive = true;           // Ativa o controle novamente
  }
}


//************************************************************************ FIM CONTROLL TEMP **************************************************************************
//************************************************************************ MENU **************************************************************************

bool menuTemp = 0;
int plusMenuTemp = 1;
int plusTimeMenuTemp = 0;
bool TempOn = 0;
int notplus = 0;
int plusTemp = 1;

void plusTempMenu() { // CurrentProgram==4 PERSONALIZADA Lembrar ao receber pelo tb

  indexProgram = true;


  resetMenuItemsProg(2);
  if (!menuTemp) {
    plusProgram = plus + plusProgram;
    Serial.println();

    if (plusProgram < 1) {
      plusProgram = 3;
    }
    if (plusProgram > 3) {
      plusProgram = 1;
    }
  }

  if (menuTemp) {
    notplus = 1;
    if (preSelect == 1) {
      savetemp = plusTemp + savetemp;
      if (savetemp <= 1) {
        savetemp = 0;
      }
      if (savetemp > 40) {
        savetemp = 0;
      }
      selectOption(2, 1);
    }
  }


  if (!menuTemp) {
    notplus = 0;
    switch (plusProgram) {

      case 1:
        selectItemMenuProg(2, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        preSelect = 1;
        break;

      case 2:
        preSelect = 2;
        selectItemMenuProg(2, 2);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

      case 3:
        preSelect = 3;
        selectItemMenuProg(2, 3);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

    }

  }
}



bool indexMenuSelect = false;

void updateMenu() {

  indexMenuSelect = true;

  if (menu < 1) {
    menu = 9;
  } else if (menu > 9) { // Ajuste conforme necessário
    menu = 1;
  }

  switch (menu) {
      clearArea(0, 0, 240, 240, TFT_BLACK);
      //drawMenu();
      resetMenuItems(0);
    case 1:
      Serial.println ("Akimentar");
      selectItemMenu(0, 1);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      break;
    case 2:
      Serial.println ("Iluinar");
      selectItemMenu(0, 2);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      break;
    case 3:
      Serial.println ("Temoeratura");
      selectItemMenu(0, 3);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      break;
    case 4:
      Serial.println ("Rotacionar");
      selectItemMenu(0, 4);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      break;
    case 5:
      Serial.println ("Wifi");
      selectItemMenu(0, 5);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      break;
    case 6:
      Serial.println ("Sobre");
      selectItemMenu(0, 6);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      break;
    case 7:
      Serial.println ("QrcOde");
      selectItemMenu(0, 7);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      break;
    case 8:
      Serial.println ("Fabrica");
      selectItemMenu(0, 8);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      break;
    case 9:
      Serial.println ("Sair");
      selectItemMenu(0, 9);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      break;
  }
}


bool menuFeeder = 0;
int plusMenuFeeder = 1;
int plusTimeMenuFeeder = 0;
bool feederOn = 0;
int plusFeerder = 1;
int plusIlumination = 1;
int plusIluminationMin = 5;

void menuProgram() { // CurrentProgram==4 PERSONALIZADA Lembrar ao receber pelo tb
  Serial.println("menuProgram");
  indexProgram = true;
  resetMenuItemsProg(1);

  if (!menuFeeder) {
    plusProgram = plus + plusProgram;
    Serial.println("!menuFeeder");

    if (plusProgram < 1) {
      plusProgram = 5;
    }
    if (plusProgram > 5) {
      plusProgram = 1;
    }
  }

  if (menuFeeder) {
    notplus = 1;
    vTaskDelay(pdMS_TO_TICKS(100));
    Serial.println("notplus menu");

    if (preSelect == 1) {
      plusMenuFeeder = plusMenuFeeder + plusFeerder;
      Serial.println("preSelect menu");

      if (plusMenuFeeder < 1) {
        plusMenuFeeder = 2;
      }
      if (plusMenuFeeder > 2) {
        plusMenuFeeder = 1;
      }
      if (plusMenuFeeder == 1) {
        feederMode = "  ATIVADO";
        feederOn = 1;
      }
      if (plusMenuFeeder == 2) {
        feederMode = "DESATIVADO";
        feederOn = 0;
      }
      selectOption(1, 1);
    }

    if (preSelect == 2) {
      feedingHour = feedingHour + plusFeerder;
      if (feedingHour < 1) {
        feedingHour = 24;
      }
      if (feedingHour > 24) {
        feedingHour = 1;
      }
      selectOption(1, 2);
    }

    if (preSelect == 3) {
      clearArea(160, 95, 30, 30, TFT_BLACK);
      feedingMinute = feedingMinute + plusFeerder;

      if (feedingMinute < 0) {
        feedingMinute = 59;
      }
      if (feedingMinute > 59) {
        feedingMinute = 0;
      }
      selectOption(1, 3);
    }
  }


  if (!menuFeeder) {
    notplus = 0;
    switch (plusProgram) {

      case 1:
        selectItemMenuProg(1, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        preSelect = 1;
        break;

      case 2:
        preSelect = 2;
        selectItemMenuProg(1, 2);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

      case 3:
        preSelect = 3;
        selectItemMenuProg(1, 3);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

      case 4:
        preSelect = 4;
        selectItemMenuProg(1, 4);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

      case 5:
        Serial.print("Sair");
        selectItemMenuProg(1, 5);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

    }
  }
}


void menuProgramOut() { // CurrentProgram==4 PERSONALIZADA Lembrar ao receber pelo tb
  Serial.println("menuProgram GOOUT");
  indexProgram = true;
  resetMenuItemsProg(1);

  if (!menuFeeder) {
    //plusProgram = plus + plusProgram;
    Serial.println("!menuFeeder");

    if (plusProgram < 1) {
      plusProgram = 5;
    }
    if (plusProgram > 5) {
      plusProgram = 1;
    }
  }

  if (menuFeeder) {
    notplus = 1;
    vTaskDelay(pdMS_TO_TICKS(100));
    Serial.println("notplus menu");

    if (preSelect == 1) {
      //plusMenuFeeder = plusMenuFeeder + plusFeerder;
      Serial.println("preSelect menu");

      if (plusMenuFeeder < 1) {
        plusMenuFeeder = 2;
      }
      if (plusMenuFeeder > 2) {
        plusMenuFeeder = 1;
      }
      if (plusMenuFeeder == 1) {
        feederMode = "  ATIVADO";
        feederOn = 1;
      }
      if (plusMenuFeeder == 2) {
        feederMode = "DESATIVADO";
        feederOn = 0;
      }
      selectOption(1, 1);
    }

    if (preSelect == 2) {
      //feedingHour = feedingHour + plusFeerder;
      if (feedingHour < 1) {
        feedingHour = 24;
      }
      if (feedingHour > 24) {
        feedingHour = 1;
      }
      selectOption(1, 2);
    }

    if (preSelect == 3) {
      clearArea(160, 95, 30, 30, TFT_BLACK);
      // feedingMinute = feedingMinute + plusFeerder;

      if (feedingMinute < 0) {
        feedingMinute = 59;
      }
      if (feedingMinute > 59) {
        feedingMinute = 0;
      }
      selectOption(1, 3);
    }
  }


  if (!menuFeeder) {
    notplus = 0;
    switch (plusProgram) {

      case 1:
        selectItemMenuProg(1, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        preSelect = 1;
        break;

      case 2:
        preSelect = 2;
        selectItemMenuProg(1, 2);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

      case 3:
        preSelect = 3;
        selectItemMenuProg(1, 3);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

      case 4:
        preSelect = 4;
        selectItemMenuProg(1, 4);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

      case 5:
        Serial.print("Sair");
        selectItemMenuProg(1, 5);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

    }
  }
}

bool menuLight = 0;
int pluLight = 1;
bool lightOn = 0;
int indexFeeder;
int preSelectLight = 1;
int plusMenuLight = 0;
void selectLightingTime() { // CurrentProgram==4 PERSONALIZADA Lembrar ao receber pelo tb

  indexProgram = true;
  resetMenuItemsProg(0);

  if (!menuLight) {
    plusProgram = plus + plusProgram;

    if (plusProgram < 1) {
      plusProgram = 7;
    }
    if (plusProgram > 7) {
      plusProgram = 1;
    }
  }
  if (menuLight) {
    notplus = 1;
    vTaskDelay(pdMS_TO_TICKS(100));

    if (preSelectLight == 1) {
      plusMenuLight = plusMenuLight + plusIlumination;

      if (plusMenuLight < 1) {
        plusMenuLight = 2;
      }
      if (plusMenuLight > 2) {
        plusMenuLight = 1;
      }
      if (plusMenuLight == 1) {
        lightOn = 1;
        statusIlum = "  ATIVADO";
      }
      if (plusMenuLight == 2) {
        statusIlum = "DESATIVADO";
        lightOn = 0;
      }
      selectOption(0, 1);
    }

    if (preSelectLight == 2) {
      illuminationStartHour = illuminationStartHour + plusIlumination;

      if (illuminationStartHour < 1) {
        illuminationStartHour = 24;
      }
      if (illuminationStartHour > 24) {
        illuminationStartHour = 1;
      }
      selectOption(0, 2);

    }

    if (preSelectLight == 3) {
      illuminationStartMinute = illuminationStartMinute + plusIluminationMin;

      if (illuminationStartMinute < 0) {
        illuminationStartMinute = 59;
      }
      if (illuminationStartMinute > 59) {
        illuminationStartMinute = 0;
      }

      selectOption(0, 3);
    }

    if (preSelectLight == 4) {

      illuminationEndHour = illuminationEndHour + plusIlumination;

      if (illuminationEndHour < 1) {
        illuminationEndHour = 24;
      }
      if (illuminationEndHour > 24) {
        illuminationEndHour = 1;
      }
      selectOption(0, 4);
    }

    if (preSelectLight == 5) {
      illuminationEndMinute = illuminationEndMinute + plusIluminationMin;

      if (illuminationEndMinute < 0) {
        illuminationEndMinute = 59;
      }
      if (illuminationEndMinute > 59) {
        illuminationEndMinute = 0;
      }
      selectOption(0, 5);
    }
  }


  if (!menuLight) {
    notplus = 0;

    switch (plusProgram) {
      case 1:
        selectItemMenuProg(0, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        preSelectLight = 1;

        break;

      case 2:
        preSelectLight = 2;
        selectItemMenuProg(0, 2);

        vTaskDelay(pdMS_TO_TICKS(100));
        break;
      case 3:
        preSelectLight = 3;
        selectItemMenuProg(0, 3);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;
      case 4:
        preSelectLight = 4;
        selectItemMenuProg(0, 4);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;
      case 5:
        preSelectLight = 5;
        selectItemMenuProg(0, 5);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

      case 6:
        preSelectLight = 6;
        selectItemMenuProg(0, 6);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

      case 7:
        Serial.print("Sair");
        selectItemMenuProg(0, 7);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;
    }
  }
}


void selectLightingTimeOut() { // CurrentProgram==4 PERSONALIZADA Lembrar ao receber pelo tb

  indexProgram = true;
  resetMenuItemsProg(0);

  if (!menuLight) {
    //plusProgram = plus + plusProgram;

    if (plusProgram < 1) {
      plusProgram = 7;
    }
    if (plusProgram > 7) {
      plusProgram = 1;
    }
  }
  if (menuLight) {
    notplus = 1;
    vTaskDelay(pdMS_TO_TICKS(100));

    if (preSelectLight == 1) {
      //plusMenuLight = plusMenuLight + plusIlumination;

      if (plusMenuLight < 1) {
        plusMenuLight = 2;
      }
      if (plusMenuLight > 2) {
        plusMenuLight = 1;
      }
      if (plusMenuLight == 1) {
        lightOn = 1;
        statusIlum = "  ATIVADO";
      }
      if (plusMenuLight == 2) {
        statusIlum = "DESATIVADO";
        lightOn = 0;
      }
      selectOption(0, 1);
    }

    if (preSelectLight == 2) {
      //illuminationStartHour = illuminationStartHour + plusIlumination;

      if (illuminationStartHour < 1) {
        illuminationStartHour = 24;
      }
      if (illuminationStartHour > 24) {
        illuminationStartHour = 1;
      }
      selectOption(0, 2);

    }

    if (preSelectLight == 3) {
      //illuminationStartMinute = illuminationStartMinute + plusIlumination;

      if (illuminationStartMinute < 0) {
        illuminationStartMinute = 59;
      }
      if (illuminationStartMinute > 59) {
        illuminationStartMinute = 0;
      }

      selectOption(0, 3);
    }

    if (preSelectLight == 4) {

      // illuminationEndHour = illuminationEndHour + plusIlumination;

      if (illuminationEndHour < 1) {
        illuminationEndHour = 24;
      }
      if (illuminationEndHour > 24) {
        illuminationEndHour = 1;
      }
      selectOption(0, 4);
    }

    if (preSelectLight == 5) {
      //illuminationEndMinute = illuminationEndMinute + plusIlumination;

      if (illuminationEndMinute < 0) {
        illuminationEndMinute = 59;
      }
      if (illuminationEndMinute > 59) {
        illuminationEndMinute = 0;
      }
      selectOption(0, 5);
    }
  }


  if (!menuLight) {
    notplus = 0;

    switch (plusProgram) {
      case 1:
        selectItemMenuProg(0, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        preSelectLight = 1;

        break;

      case 2:
        preSelectLight = 2;
        selectItemMenuProg(0, 2);

        vTaskDelay(pdMS_TO_TICKS(100));
        break;
      case 3:
        preSelectLight = 3;
        selectItemMenuProg(0, 3);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;
      case 4:
        preSelectLight = 4;
        selectItemMenuProg(0, 4);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;
      case 5:
        preSelectLight = 5;
        selectItemMenuProg(0, 5);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

      case 6:
        preSelectLight = 6;
        selectItemMenuProg(0, 6);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

      case 7:
        Serial.print("Sair");
        selectItemMenuProg(0, 7);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;
    }
  }
}

void executeAction() {
  if (firstPression == true) {
    switch (menu) {
      case 1:
        menuProgram();
        break;
      case 2:
        selectLightingTime();
        break;
      case 3:
        plusTempMenu();
        break;
      case 4:
        rotationDisplay();
        break;
      case 5:
        ResetWifi();
        break;
      case 6:
        showAbout(VERSION);
        break;
      case 7:
        showQr();
        break;
      case 8:
        restoreDefaults();
        break;
      case 9:
        clearArea(0, 0, 240, 240, TFT_BLACK);
        menuGoOut();
        break;
    }
  }
}


int plusReset = 1;

void  ResetWifi() {
  plusReset = plus + plusReset;
  if (plusReset < 1) {
    plusReset = 2;
  }
  if (plusReset > 2) {
    plusReset = 1;
  }

  resetScreen();

  switch (plusReset) {
    case 1:
      Serial.print("Reset ");
      selectItemMenuReset(1, 1);
      vTaskDelay(pdMS_TO_TICKS(100));
      break;
    case 2:
      Serial.print("Sair");
      selectItemMenuReset(1, 2);
      vTaskDelay(pdMS_TO_TICKS(100));
      break;
  }
}

int plusDefaults = 1;

void  restoreDefaults() {

  plusDefaults = plus + plusDefaults;
  Serial.println();
  if (plusDefaults < 1) {
    plusDefaults = 2;
  }
  if (plusDefaults > 2) {
    plusDefaults = 1;
  }

  defaultsScreen();

  switch (plusDefaults) {
    case 1:
      Serial.print("Restaurar ");
      selectItemMenuDefaults(1, 1);
      vTaskDelay(pdMS_TO_TICKS(100));
      break;
    case 2:
      Serial.print("Sair");
      selectItemMenuDefaults(1, 2);
      vTaskDelay(pdMS_TO_TICKS(100));
      break;
  }
}


int plusRotation = 0;
int rotationMemory = 0;

void  rotationDisplay() {

  //indexProgram = true;
  plusRotation = plus + plusRotation;
  Serial.println();
  if (plusRotation < 1) {
    plusRotation = 3;
  }
  if (plusRotation > 3) {
    plusRotation = 1;
  }

  Serial.println("preSelect");
  Serial.println(plusRotation);
  rotationScreen();

  switch (plusRotation) {
    case 1:
      Serial.print("Horizontal ");
      selectItemMenuRotation(1, 1);
      rotationMemory = 2;
      vTaskDelay(pdMS_TO_TICKS(100));
      break;

    case 2:
      Serial.print("Torre");
      selectItemMenuRotation(1, 2);
      rotationMemory = 3;
      vTaskDelay(pdMS_TO_TICKS(100));
      break;
    case 3:
      Serial.print("Sair");
      selectItemMenuRotation(1, 3);
      vTaskDelay(pdMS_TO_TICKS(100));
      break;
  }
}


void showQr() {
  showQrCode();
}
SemaphoreHandle_t xSemaphore;

void menuGoOut() {
  // if (xSemaphoreTake(xSemaphore, 5000 / portTICK_PERIOD_MS) == pdTRUE) {

  goIn = false;
  menu = 0;
  FlagBUp = 0;
  FlagBDown = 0;
  menuTemp = 0;
  menuLight = 0;
  menuFeeder = 0;
  forceClockRedraw = true;

  Serial.println("Saindo");
  //clearArea(0, 240, 240, 240, TFT_BLACK);

  firstPression = false;

  return;
  //   xSemaphoreGive(xSemaphore);

  // }
}

//************************************************************************ FIM MENU **************************************************************************
//************************************************************************ INTERRUPT **************************************************************************

void interruptUp() {
  delayMicroseconds(100);
  portENTER_CRITICAL_ISR(&mux); // início da seção crítica
  if (!digitalRead(buttonUp)) FlagBUp = 1;
  portEXIT_CRITICAL_ISR(&mux); // fim da seção crítica
}// FIM interruptUp()

void interruptDown() {
  delayMicroseconds(100);
  portENTER_CRITICAL_ISR(&mux); // início da seção crítica
  if (!digitalRead(buttonDown)) FlagBDown = 1;
  portEXIT_CRITICAL_ISR(&mux); // fim da seção crítica
}// FIM interruptUp()


//************************************************************************ FIM INTERRUPT **************************************************************************
//************************************************************************ CLOCK **************************************************************************
int rtcHour ;
int rtcMinutes ;
char bufferTime[6]; // Buffer de 6 bytes para "HH:MM\0"

void digitalClockDisplay() {
  time_t now = myRTC.get();  // Obtém o tempo atual do RTC
  tmElements_t tm;
  breakTime(now, tm);  // Decompõe o tempo em componentes
}

void printDigits(int digits) {
  Serial.print(':');
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

//************************************************************************ FIM CLOCK **************************************************************************
//************************************************************************ CHECK ERROR **************************************************************************
void checkError() {
  /*
      current1 = ina219_1.getCurrent_mA();
      current2 = ina219_2.getCurrent_mA();
      current3 = ina219_3.getCurrent_mA();
      current5 = ina219_5.getCurrent_mA();

      // Exibe os valores no monitor serial
      Serial.print("Corrente 1 AQUECEDDOR: "); Serial.print(current1); Serial.println(" mA");
      Serial.print("Corrente 2 LAMPADA 1: "); Serial.print(current2); Serial.println(" mA");
      Serial.print("Corrente 3 LAMPADA 2: "); Serial.print(current3); Serial.println(" mA");
      Serial.print("Corrente 5 COOLER: "); Serial.print(current5); Serial.println(" mA");


    for (int i = 0; i <= 5; i++) {
    if (valueError != -1) {
      moduloError[i] = mut.readPcfBuffer(0x38, i);
    } else {
      moduloError[i] = -1; // Definir valor de erro
    }
    }

    if (intensityLight1 == 0 && moduloError[0] == 0) {
    controllError[0] = 0;
    } else if (intensityLight1 != 0 && moduloError[0] == 1) {
    controllError[0] = intensityLight1 * 10;
    } else {
    controllError[0] = -1;
    }
    if (intensityLight2 == 0 && moduloError[1] == 0) {
    controllError[1] = 0;
    } else if (intensityLight2 != 0 && moduloError[1] == 1) {
    controllError[1] = intensityLight2 * 10;
    } else {
    controllError[1] = -1;
    }
    if (intensityLight3 == 0 && moduloError[2] == 0) {
    controllError[2] = 0;
    } else if (intensityLight3 != 0 && moduloError[2] == 1) {
    controllError[2] = intensityLight3 * 10;
    } else {
    controllError[2] = -1;
    }

    if (digitalRead(cooler) == 0 && moduloError[3] == 0) {
    controllError[3] = 0;
    } else if (digitalRead(cooler) == 1 && moduloError[3] == 1) {
    controllError[3] = 1;
    } else {
    controllError[3] = -1;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);

    if (digitalRead(feeder) == 0 && moduloError[4] == 0) {
    controllError[4] = 0;
    } else if (digitalRead(feeder) == 1 && moduloError[4] == 1) {
    controllError[4] = 1;
    } else {
    controllError[4] = -1;
    }

    if (medirCorrente(1, ADS1) < 800 ) {
      controllError[5] = 0;
    } else if (medirCorrente(1, ADS1) >= 800 && medirCorrente(1, ADS1) <= 1500) {
      controllError[5] = 1;
    } else {
      controllError[5] = -1;
    }




    //monitorError();

    //delay(2000);
  */

  controllError[0] = intensityLight1 * 10;
  controllError[1] = intensityLight2 * 10;
  controllError[2] = intensityLight3 * 10;
  controllError[3] = digitalRead(cooler);
  controllError[4] = digitalRead(feeder);
  controllError[5] = digitalRead(heater);
}

void monitorError () {
  Serial.println("--------------------------------------------------------------LEITURA MULTIPLEXADOR --------------------------------------------------------------");
  Serial.print("L1: ");
  Serial.println(moduloError[0]);
  Serial.print("L2: ");
  Serial.println(moduloError[1]);
  Serial.print("L3: ");
  Serial.println(moduloError[2]);
  Serial.print("Cooler: ");
  Serial.println(moduloError[3]);
  Serial.print("Feeder: ");
  Serial.println(moduloError[4]);
  Serial.print("Aquecedor: ");
  Serial.println(moduloError[5]);
  Serial.print("Digital REad Feeder: ");
  Serial.println(digitalRead(feeder));


  Serial.println("--------------------------------------------------------------oQUE SERA ENVIADO --------------------------------------------------------------");


  Serial.print("L1: ");
  Serial.println(controllError[0]);
  Serial.print("L2: ");
  Serial.println(controllError[1]);
  Serial.print("L3: ");
  Serial.println(controllError[2]);

  Serial.print("Cooler: ");
  Serial.println(controllError[3]);
  Serial.print("Feeder: ");
  Serial.println(controllError[4]);
  Serial.print("Aquecedor: ");
  Serial.println(controllError[5]);
}
//************************************************************************FIM CHECK ERROR **************************************************************************
//************************************************************************ PRE-PROGRAM **************************************************************************

time_t getTimestamp(int hour, int minute, int dayOffset = 0) {
  tmElements_t tm;
  myRTC.read(tm);
  tm.Hour = hour;
  tm.Minute = minute;
  tm.Second = 0;
  tm.Day += dayOffset;
  return makeTime(tm);
}

void checkFeedingTimes(int hour1, int hour2) {
  time_t now = myRTC.get();

  time_t feedTime1 = getTimestamp(hour1, 0);
  time_t feedTime2 = getTimestamp(hour2, 0);
  time_t lastFeed, nextFeed;

  if (now > feedTime1 && now > feedTime2) {
    // Caso 1: Ambas horas já passaram
    lastFeed = feedTime1;                   // Última alimentação foi na hora1
    nextFeed = getTimestamp(hour2, 0, 1);   // Próxima será na hora2, no dia seguinte
  } else if (now > feedTime2 && now < feedTime1) {
    // Caso 2: Hora2 passou, mas hora1 não
    lastFeed = feedTime2;                   // Última alimentação foi na hora2
    nextFeed = feedTime1;                   // Próxima será na hora1 de hoje
  } else {
    // Caso 3: Hora2 ainda não passou
    lastFeed = getTimestamp(hour1, 0, -1);  // Última alimentação foi na hora1, no dia anterior
    nextFeed = feedTime2;                   // Próxima será na hora2 de hoje
  }

  // Exibe os resultados
  Serial.print("Last feeding: ");
  last =  static_cast<unsigned long long>(lastFeed);  // Converte para long long
  Serial.println(last);
  Serial.print("Next feeding: ");
  next =  static_cast<unsigned long long>(nextFeed);  // Converte para long long
  Serial.println(next);
}
//************************************************************************  CHECK  ****************************************************************************





unsigned long teste = 0;

void checkAll() {// && !lampImmediateMqtt && !lampRoutineMqtt && !feederRoutineMqtt && !feederNowMqtt && !tempAdjustMqtt && !timeAjustMqtt &&  !signatureAdjustMqtt
  bool hasChanged = false; // Variável para indicar se houve mudança
  bool hasChangedLight = false;
  bool hasChangedFeeder = false;
  bool hasChangedTemp = false;

  for (int i = 0; i <=  2; i++) { // Itera sobre as 3 posições do vetor

    if (controllError[i] != lastValue[i] ) { //&& (teste - lastButtonIntensity >= stabilizationIntensity )
      lastValue[i] = controllError[i]; // Atualiza a última intensidade registrada
      hasChangedLight = true;
      Serial.println("Indice alterado");
      Serial.println(i);

    }
  }

  if (controllError[4] != lastValue[4] && (teste - lastButtonIntensity >= stabilizationIntensity )) {
    lastValue[4] = controllError[4]; // Atualiza a última intensidade registrada
    hasChangedFeeder = true;
    Serial.println("Indice alterado");
    Serial.println(4);
  }

  if (controllError[3] != lastValue[3] && (teste - lastButtonIntensity >= stabilizationIntensity )) {
    lastValue[3] = controllError[3]; // Atualiza a última intensidade registrada
    hasChangedTemp = true;
    Serial.println("Indice alterado");
    Serial.println(3);
  }

  if (controllError[5] != lastValue[5] && (teste - lastButtonIntensity >= stabilizationIntensity )) {
    lastValue[5] = controllError[5]; // Atualiza a última intensidade registrada
    hasChangedTemp = true;
    Serial.println("Indice alterado");
    Serial.println(5);
  }

  if (hasChangedLight) {
    sendDataL1();
    sendDataL2();
    sendDataL3();
  }
  if (hasChangedTemp) {
    sendDataCooler();
    sendDataHeater();
    sendDataTempControll();
    sendDataTemp();
  }
}
void ResetDefaut() {

  Serial.println("Resetando configuracoes do dispositivo...");

  // -------------------------------------------------
  // 1️⃣ LIMPAR PREFERENCES (Config do Device)
  // -------------------------------------------------

  prefs.begin("device", false);
  prefs.clear();     // Apaga TODAS as chaves do namespace
  prefs.end();

  Serial.println("Preferences apagadas.");

  // -------------------------------------------------
  // 2️⃣ LIMPAR ROTINAS NA EEPROM
  // -------------------------------------------------

  EEPROM.begin(EEPROM_SIZE);

  // Limpar LampRoutine
  for (int i = 0; i < MAX_LAMP_ROUTINES; i++) {
    LampRoutine emptyRoutine = {0, 0, 0, 0, 0};
    EEPROM.put(EEPROM_LAMP_ROUTINES + i * sizeof(LampRoutine), emptyRoutine);
  }

  // Limpar FeederRoutine
  for (int i = 0; i < MAX_FEEDER_ROUTINES; i++) {
    FeederRoutine emptyRoutine = {0, 0, 0};
    EEPROM.put(EEPROM_FEEDER_ROUTINES + i * sizeof(FeederRoutine), emptyRoutine);
  }

  EEPROM.commit();

  Serial.println("Rotinas apagadas.");

  // -------------------------------------------------
  // 3️⃣ RESETAR FLAGS DA MemoryManagment (WiFi)
  // -------------------------------------------------

  setReconnect = false;
  memory.saveMemoryBool("reconnect", setReconnect);
  EEPROM.commit();

  Serial.println("Reconnect resetado.");

  // -------------------------------------------------
  // 4️⃣ REINICIAR
  // -------------------------------------------------

  Serial.println("Reset concluido. Reiniciando...");
  ESP.restart();
}

void routineDisplay() {
  if (statusIlum == "  ATIVADO") {
    controllLed( illuminationStartHour, illuminationStartMinute, illuminationEndHour, illuminationEndMinute);

    Serial.print("Iicio Iluminação: ");
    Serial.print(illuminationStartHour);
    Serial.println(": ");
    Serial.print(illuminationStartMinute);

    Serial.print("Fim Iluminação: ");
    Serial.print(illuminationEndHour);
    Serial.println(": ");
    Serial.print(illuminationEndMinute);
  }
  if (routineIlu) {

    lampRoutine();
  }
  if (feederMode == "  ATIVADO") {
    controllFood(feedingHour, feedingMinute, 1);
    Serial.print("Proxima alimentação: ");
    Serial.print(feedingHour);
    Serial.println(": ");
    Serial.print(feedingMinute);
  }

  if (routineFeeder) {

    feederRoutine();
  }
}




//************************************************************************ FIM CHECK **************************************************************************
//************************************************************************ FIM PRE-PROGRAM **************************************************************************
void controllButton( void *param) {//void *param
  while (1) {
    bool statusDown = digitalRead(buttonDown);
    bool statusUp = digitalRead(buttonUp);
    lastButtonIntensity = millis();
    unsigned int contDouble = 0;
    bool enterDouble = false;

    if ((statusDown == LOW || statusUp == LOW) and firstPression == true) {
      lastMenuPress = millis();
    }


    if (statusDown == LOW && statusUp == LOW) {  // Verifica se os dois botões estão pressionados
      while (!enterDouble && (contDouble < 100)) {
        if (!digitalRead(buttonUp) && !digitalRead(buttonDown)) {  // Verifica novamente se os dois botões ainda estão pressionados
          contDouble += 10;
          vTaskDelay(200 / portTICK_PERIOD_MS);
          if (contDouble >= 100) {  // Após 2 segundos de ambos os botões pressionados
            if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
              firstPression = true;  // Protegido
              menu = 1;
              updateMenu();
              xSemaphoreGive(xSemaphore);  // Libera o semáforo

              vTaskDelay(pdMS_TO_TICKS(500));
            }
          }
        } else {
          break;
        }
      }
    }

    else if (FlagBUp) {
      portENTER_CRITICAL_ISR(&mux); // início da seção crítica
      FlagBUp = 0;
      portEXIT_CRITICAL_ISR(&mux); // fim da seção crítica
      lastButtonIntensity = millis();
      if (statusUp == LOW) { // Se o botão está pressionado (ajuste conforme a lógica do seu botão)
        unsigned int aux = 0;
        bool enter = false;
        while (!enter && (aux < 200)) {
          if (!digitalRead(buttonUp)) { // Se o botão ainda está sendo pressionado
            aux += 10;
            vTaskDelay(100 / portTICK_PERIOD_MS);
            if (aux >= 200) {
              enter = true;
              vTaskDelay(100 / portTICK_PERIOD_MS);

              if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
                xSemaphoreGive(xSemaphore);

                if (intensityLight != 0  && !firstPression && menu == 0) { // Se o botão ainda nãp foi pressionado e caso a intensidade possui um valor doferente de zero apagar as lampadas
                  Serial.println("zerando");
                  intensityLight1 = intensityLight2 = intensityLight3 = intensityLight = 0;
                  updateLightIntensity(light1, 0);
                  updateLightIntensity(light2, 0);
                  updateLightIntensity(light3, 0);
                  lightFooter = false;
                  vTaskDelay(200 / portTICK_PERIOD_MS);
                  clearArea(0, 60, 240, 80, TFT_BLACK);
                  showLamp(0, "0"); //  Verificar com a Bruna
                  drawFooter(coolerFooter, heaterFooter, feederFooter, lightFooter);

                }
                else if (firstPression && !goIn && menu != 0) {
                  Serial.println("firstPression && !goIn && menu != 0");
                  if (menu == 4) {
                    idealTemp = savetemp;
                  }
                  Serial.println("firstPression && !goIn && menu != 0 execute");

                  executeAction();
                  if (menu != 0) {
                    goIn = true;
                  }
                  xSemaphoreGive(xSemaphore);

                  break;
                }
              }


              if (menu == 1 && goIn ) {
                if (xSemaphoreTake(xSemaphore, 5000 / portTICK_PERIOD_MS) == pdTRUE) {
                  menuFeeder = !menuFeeder;
                  Serial.println("menu == 1 && goIn");
                  if (plusProgram == 1) {
                    Serial.println("plusProgram");
                    selectOption(1, 1);
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    menuProgramOut();
                    // vTaskDelay(1000 / portTICK_PERIOD_MS);

                  }
                  if (plusProgram == 2) {
                    selectOption(1, 2);
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    menuProgramOut();
                  }
                  if (plusProgram == 3) {
                    selectOption(1, 3);
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    menuProgramOut();
                  }
                  if (plusProgram == 4 )
                  {
                    if (feederMode == "  ATIVADO") {
                      routineFeeder = 0;
                      sendfeederRoutineConfig(1, feedingHour, feedingMinute);
                      prefs.putBool("routineFeeder", routineFeeder);
                    }
                    else if (feederMode == "DESATIVADO") {
                      sendDeleteFeeder();
                    }
                    menuFeeder = 0;
                    goIn = false;
                    updateMenu();

                    prefs.putInt("feedMin", feedingMinute);
                    prefs.putInt("feedHour", feedingHour);
                    prefs.putBool("feederOn", feederOn);

                    Serial.println(feedingMinute);
                    Serial.println(feedingHour);
                    Serial.println(feederOn);

                    lastfeederOn = feederOn;
                    lastfeedingHour = feedingHour;
                    lastfeedingMinute = feedingMinute;

                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                  }
                  if (plusProgram == 5 ) {
                    goIn = false;
                    menuFeeder = 0;

                    feederOn = lastfeederOn;
                    feedingHour = lastfeedingHour;
                    feedingMinute = lastfeedingMinute;

                    updateMenu();
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                  }
                }
              }

              if (menu == 2 && goIn ) {
                if (xSemaphoreTake(xSemaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
                  menuLight = !menuLight;
                  if (plusProgram == 1) {
                    selectOption(0, 1);
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(300 / portTICK_PERIOD_MS);
                    selectLightingTimeOut();
                  }

                  if (plusProgram == 2) {
                    selectOption(0, 2);
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(300 / portTICK_PERIOD_MS);
                    selectLightingTimeOut();
                  }
                  if (plusProgram == 3) {
                    selectOption(0, 3);
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(300 / portTICK_PERIOD_MS);
                    selectLightingTimeOut();
                  }
                  if (plusProgram == 4) {
                    selectOption(0, 4);
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(300 / portTICK_PERIOD_MS);
                    selectLightingTimeOut();
                  }
                  if (plusProgram == 5) {
                    selectOption(0, 5);
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(300 / portTICK_PERIOD_MS);
                    selectLightingTimeOut();
                  }
                  if (plusProgram == 6 ) {

                    if (statusIlum == "  ATIVADO") {
                      routineIlu = 0;
                      sendLampRoutine(illuminationStartHour, illuminationStartMinute, illuminationEndHour, illuminationEndMinute);
                      prefs.putBool("routineLight", routineIlu);
                      Serial.println(routineIlu);
                    }
                    else if (statusIlum == "DESATIVADO") {
                      sendDeleteIlumination();
                    }
                    menuLight = 0;
                    goIn = false;
                    updateMenu();
                    prefs.putBool("lightOn", lightOn);

                    prefs.putInt("lightStartH", illuminationStartHour);
                    prefs.putInt("lightStartM", illuminationStartMinute);

                    prefs.putInt("lightEndH", illuminationEndHour);
                    prefs.putInt("lightEndM", illuminationEndMinute);
                    vTaskDelay(500 / portTICK_PERIOD_MS);

                    lastilluminationEndHour = illuminationEndHour;
                    lastilluminationEndMinute = illuminationEndMinute;
                    lastilluminationStartHour = illuminationStartHour;
                    lastilluminationStartMinute = illuminationStartMinute;
                    laststatusIlum = statusIlum;

                    xSemaphoreGive(xSemaphore);

                  }
                  if (plusProgram == 7 ) {

                    illuminationEndHour = lastilluminationEndHour;
                    illuminationEndMinute = lastilluminationEndMinute;
                    illuminationStartHour = lastilluminationStartHour;
                    illuminationStartMinute = lastilluminationStartMinute;
                    statusIlum = laststatusIlum;

                    goIn = false;
                    menuLight = 0;
                    updateMenu();
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                  }
                }
              }
              if (menu == 3 && goIn ) {

                if (xSemaphoreTake(xSemaphore, 5000 / portTICK_PERIOD_MS) == pdTRUE) {
                  menuTemp = !menuTemp;

                  if (plusProgram == 1) {
                    selectOption(2, 1);
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    // plusTempMenu();
                    xSemaphoreGive(xSemaphore);
                  }

                  if (plusProgram == 2 ) {
                    menuTemp = 0;

                    goIn = false;
                    updateMenu();
                    idealTemp = savetemp;
                    modificTempDTemp = savetemp;
                    modificTempP = true;
                    range = 1;
                    sendDataTempControllConfig();


                    prefs.putInt("tempIdeal", modificTempDTemp);
                    vTaskDelay(100 / portTICK_PERIOD_MS);

                    prefs.putBool("tempModified", modificTempP);
                    vTaskDelay(100 / portTICK_PERIOD_MS);

                    prefs.putFloat("tempRange", range);
                    vTaskDelay(100 / portTICK_PERIOD_MS);

                    Serial.println(modificTempDTemp);


                    vTaskDelay(100 / portTICK_PERIOD_MS);

                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(100 / portTICK_PERIOD_MS);



                  }
                  if (plusProgram == 3 ) {

                    plus = 0;
                    savetemp = idealTemp;
                    goIn = false;
                    updateMenu();
                    menuTemp = 0;
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(100 / portTICK_PERIOD_MS);

                  }
                }
              }
              if (menu == 4 && goIn) {
                if (xSemaphoreTake(xSemaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
                  if (plusRotation == 1 || plusRotation == 2) {

                    prefs.putInt("rotation", rotationMemory);
                    Serial.println(rotationMemory);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                    ESP.restart();
                  }
                  else if (plusRotation == 3) {
                    goIn = false;
                    updateMenu();
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                  }
                }
              }
              if (menu == 5 && goIn ) {

                if (xSemaphoreTake(xSemaphore, 5000 / portTICK_PERIOD_MS) == pdTRUE) {
                  if (plusReset == 1) {
                    sendDataWifiReset();
                    stopEspIDFStack();

                    setReconnect = false;

                    memory.saveMemoryBool("reconnect", setReconnect);

                    Serial.print("setReconnect on button: ");
                    Serial.println(memory.readMemoryBool("reconnect", false));

                    serverInitialized = false;
                    prefs.putBool("serverInit", serverInitialized);

                    //wifiManager.resetSettings();
                    ESP.restart();
                  }
                  else if (plusReset == 2) {
                    goIn = false;
                    updateMenu();
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                  }
                }
              }

              if (menu == 6 && goIn ) {
                if (xSemaphoreTake(xSemaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
                  goIn = false;
                  updateMenu();
                  xSemaphoreGive(xSemaphore);
                  vTaskDelay(100 / portTICK_PERIOD_MS);
                }
              }
              if (menu == 7 && goIn ) {
                if (xSemaphoreTake(xSemaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
                  goIn = false;
                  updateMenu();
                  xSemaphoreGive(xSemaphore);
                  vTaskDelay(100 / portTICK_PERIOD_MS);
                }
              }
              if (menu == 8 && goIn ) {
                if (xSemaphoreTake(xSemaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
                  if (plusDefaults == 1) {
                    sendDataHardReset();
                    ResetDefaut();
                  }
                  else if (plusDefaults == 2) {
                    goIn = false;
                    updateMenu();
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                  }
                }
              }
            }
          }
          else {
            break; // Botão foi liberado antes do tempo
          }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
        if (statusUp == LOW && !enter) {

          if (firstPression == false) {
            if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {

              activateLight();

              int mediaLamp = ((intensityLight + intensityLight2 + intensityLight3) / 3) ;

              clearArea(0, 60, 240, 80, TFT_BLACK);
              showLamp(0, String(mediaLamp * 10)); //  Verificar com a Bruna
              lightFooter = true;
              drawFooter(coolerFooter, heaterFooter, feederFooter, lightFooter);
              xSemaphoreGive(xSemaphore);
            }
          }


          else if  (firstPression && !goIn ) {
            if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
              Serial.println("firstPression && !goIn ");
              menu--;
              updateMenu();
              xSemaphoreGive(xSemaphore);

            }
          }

          else if  (menu == 1  && goIn) {
            if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {
              Serial.println("menu == 1  && goIn ");

              if (notplus == 1) {
                plus = 0;
                plusFeerder = -1;
              }
              if (notplus == 0) {
                plus = -1;
              }
              // vTaskDelay(500 / portTICK_PERIOD_MS);
              executeAction();
              xSemaphoreGive(xSemaphore);
              vTaskDelay(200 / portTICK_PERIOD_MS);

            }
          }
          else if (menu == 2  && goIn ) {
            if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {
              if (notplus == 1) {
                plus = 0;
                plusIlumination = -1;
                plusIluminationMin = -5;
              }
              if (notplus == 0) {
                plus = -1;
              }
              executeAction();
              xSemaphoreGive(xSemaphore);
              vTaskDelay(200 / portTICK_PERIOD_MS);

            }
          }
          else if (menu == 3  && goIn ) {
            if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {
              if (notplus == 1) {
                plus = 0;
                plusTemp = -1;
              }
              if (notplus == 0) {
                plus = -1;
              }
              xSemaphoreGive(xSemaphore);
              executeAction();
              vTaskDelay(200 / portTICK_PERIOD_MS);

            }
          }

          else if (menu == 4  && goIn ) {
            if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {
              plus = -1;
              executeAction();
              xSemaphoreGive(xSemaphore);
              vTaskDelay(200 / portTICK_PERIOD_MS);

            }
          }
          else if (menu == 5 && goIn ) {
            if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {
              plus = -1;
              executeAction();
              xSemaphoreGive(xSemaphore);
              vTaskDelay(200 / portTICK_PERIOD_MS);
            }
          }
          else if (menu == 6 && goIn ) {
            if (xSemaphoreTake(xSemaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
              goIn = false;
              updateMenu();
              xSemaphoreGive(xSemaphore);
              vTaskDelay(100 / portTICK_PERIOD_MS);
            }
          }
          else if (menu == 7 && goIn ) {
            if (xSemaphoreTake(xSemaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
              goIn = false;
              updateMenu();
              xSemaphoreGive(xSemaphore);
              vTaskDelay(100 / portTICK_PERIOD_MS);
            }
          }
          else if (menu == 8  && goIn ) {
            if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {
              plus = -1;
              //  Serial.println("menu == 2  && goIn");
              executeAction();
              xSemaphoreGive(xSemaphore);
              vTaskDelay(200 / portTICK_PERIOD_MS);

            }
          }
        }
      }
    }
    else if (FlagBDown) {  // Botão direito

      portENTER_CRITICAL_ISR(&mux); // início da seção crítica
      FlagBDown = 0;
      portEXIT_CRITICAL_ISR(&mux); // fim da seção crítica

      if (statusDown == LOW) { // Se o botão está pressionado (ajuste conforme a lógica do seu botão)
        unsigned int aux = 0;
        bool enter = false;
        while (!enter && (aux < 200)) {
          if (!digitalRead(buttonDown)) { // Se o botão ainda está sendo pressionado
            aux += 10;
            vTaskDelay(100 / portTICK_PERIOD_MS);
            if (aux >= 200) {
              enter = true;
              vTaskDelay(100 / portTICK_PERIOD_MS);

              if (firstPression && !goIn && menu != 0) {

                if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
                  Serial.println("firstPression && !goIn && menu != 0");
                  xSemaphoreGive(xSemaphore);
                  if (menu == 4) {
                    idealTemp = savetemp;
                  }
                  executeAction();
                  Serial.println("firstPression && !goIn && menu != 0 ezecute");

                  if (menu != 0) {
                    goIn = true;
                  }
                  xSemaphoreGive(xSemaphore);

                  break;

                }
              }

              if (menu == 1 && goIn ) {
                if (xSemaphoreTake(xSemaphore, 5000 / portTICK_PERIOD_MS) == pdTRUE) {
                  menuFeeder = !menuFeeder;
                  Serial.println("menu == 1 && goIn");
                  if (plusProgram == 1) {

                    selectOption(1, 1);
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    menuProgramOut();
                    //vTaskDelay(1000 / portTICK_PERIOD_MS);
                    Serial.println("plusProgram == 1");
                  }
                  if (plusProgram == 2) {
                    selectOption(1, 2);
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    menuProgramOut();
                  }
                  if (plusProgram == 3) {
                    selectOption(1, 3);
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    menuProgramOut();
                    notplus = 1;
                  }

                  if (plusProgram == 4 ) {

                    if (feederMode == "  ATIVADO") {
                      routineFeeder = 0;
                      sendfeederRoutineConfig(1, feedingHour, feedingMinute);
                      prefs.putBool("routineFeeder", routineFeeder);
                    }
                    else if (feederMode == "DESATIVADO") {
                      sendDeleteFeeder();
                    }
                    menuFeeder = 0;

                    goIn = false;
                    updateMenu();

                    prefs.putInt("feedMin", feedingMinute);
                    prefs.putInt("feedHour", feedingHour);
                    prefs.putBool("feederOn", feederOn);

                    Serial.println(feedingMinute);
                    Serial.println(feedingHour);
                    Serial.println(feederOn);


                    lastfeederOn = feederOn;
                    lastfeedingHour = feedingHour;
                    lastfeedingMinute = feedingMinute;

                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                  }
                  if (plusProgram == 5 ) {
                    goIn = false;
                    menuFeeder = 0;
                    feederOn = lastfeederOn;
                    feedingHour = lastfeedingHour;
                    feedingMinute = lastfeedingMinute;


                    updateMenu();
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                  }
                }
              }
              if (menu == 2 && goIn ) {
                if (xSemaphoreTake(xSemaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
                  menuLight = !menuLight;

                  if (plusProgram == 1) {
                    selectOption(0, 1);
                    xSemaphoreGive(xSemaphore);
                    selectLightingTimeOut();
                    vTaskDelay(300 / portTICK_PERIOD_MS);
                  }
                  if (plusProgram == 2) {
                    selectOption(0, 2);
                    //menuFeeder = 1;
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(300 / portTICK_PERIOD_MS);
                    selectLightingTimeOut();
                  }
                  if (plusProgram == 3) {
                    selectOption(0, 3);
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(300 / portTICK_PERIOD_MS);
                    selectLightingTimeOut();
                  }

                  if (plusProgram == 4) {
                    selectOption(0, 4);
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(300 / portTICK_PERIOD_MS);
                    selectLightingTimeOut();

                  }
                  if (plusProgram == 5) {
                    selectOption(0, 5);
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(300 / portTICK_PERIOD_MS);
                    selectLightingTimeOut();
                  }
                  if (plusProgram == 6 ) {

                    if (statusIlum == "  ATIVADO") {
                      routineIlu = 0;
                      sendLampRoutine(illuminationStartHour, illuminationStartMinute, illuminationEndHour, illuminationEndMinute);
                      prefs.putBool("routineLight", routineIlu);
                      Serial.println(routineIlu);
                    }
                    else if (statusIlum == "DESATIVADO") {
                      sendDeleteIlumination();
                    }

                    menuLight = 0;
                    goIn = false;
                    updateMenu();

                    prefs.putBool("lightOn", lightOn);

                    prefs.putInt("lightStartH", illuminationStartHour);
                    prefs.putInt("lightStartM", illuminationStartMinute);

                    prefs.putInt("lightEndH", illuminationEndHour);
                    prefs.putInt("lightEndM", illuminationEndMinute);

                    Serial.println(lightOn);

                    lastilluminationEndHour = illuminationEndHour;
                    lastilluminationEndMinute = illuminationEndMinute;
                    lastilluminationStartHour = illuminationStartHour;
                    lastilluminationStartMinute = illuminationStartMinute;
                    laststatusIlum = statusIlum;

                    vTaskDelay(500 / portTICK_PERIOD_MS);
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(100 / portTICK_PERIOD_MS);


                  }
                  if (plusProgram == 7 ) {
                    goIn = false;
                    menuLight = 0;
                    updateMenu();
                    illuminationEndHour = lastilluminationEndHour;
                    illuminationEndMinute = lastilluminationEndMinute;
                    illuminationStartHour = lastilluminationStartHour;
                    illuminationStartMinute = lastilluminationStartMinute;
                    statusIlum = laststatusIlum;
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(200 / portTICK_PERIOD_MS);
                  }

                }
              }

              if (menu == 3 && goIn ) {
                //  Serial.println ("PRESSOINAMENTO LONGOOOOOOOOOOOOO");

                if (xSemaphoreTake(xSemaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
                  menuTemp = !menuTemp;

                  if (plusProgram == 1) {
                    selectOption(2, 1);
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    //plusTempMenu();

                  }

                  if (plusProgram == 2 ) {
                    menuTemp = 0;
                    goIn = false;
                    updateMenu();
                    idealTemp = savetemp;
                    modificTempDTemp = savetemp;
                    modificTempP = true;
                    range = 1;
                    sendDataTempControllConfig();

                    prefs.putInt("tempIdeal", modificTempDTemp);
                    prefs.putBool("tempModified", modificTempP);
                    prefs.putFloat("tempRange", range);

                    Serial.println(modificTempDTemp);
                    xSemaphoreGive(xSemaphore);

                  }

                  if (plusProgram == 3 ) {
                    menuTemp = 0;
                    plus = 0;
                    savetemp = idealTemp;
                    goIn = false;
                    updateMenu();
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                  }

                }

              }

              if (menu == 4 && goIn) {
                if (xSemaphoreTake(xSemaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
                  if (plusRotation == 1 || plusRotation == 2) {
                    prefs.putInt("rotation", rotationMemory);
                    Serial.println(rotationMemory);
                    vTaskDelay(100 / portTICK_PERIOD_MS);

                    ESP.restart();
                  }
                  else if (plusRotation == 3) {
                    goIn = false;
                    updateMenu();
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                  }
                }
              }
              if (menu == 5 && goIn ) {

                if (xSemaphoreTake(xSemaphore, 5000 / portTICK_PERIOD_MS) == pdTRUE) {
                  if (plusReset == 1) {
                    sendDataWifiReset();
                    stopEspIDFStack();

                    // -----------------------------
                    // 1️⃣ Reconnect (fica na EEPROM)
                    // -----------------------------

                    setReconnect = false;
                    memory.saveMemoryBool("reconnect", setReconnect);

                    Serial.print("setReconnect on button: ");
                    Serial.println(memory.readMemoryBool("reconnect", false));

                    // ------------------------------------
                    // 2 ServerInitialized (Preferences)
                    // ------------------------------------

                    serverInitialized = false;
                    prefs.putBool("serverInit", serverInitialized);
                    //wifiManager.resetSettings();
                    ESP.restart();
                  }
                  else if (plusReset == 2) {
                    goIn = false;
                    updateMenu();
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                  }
                }
              }

              if (menu == 6 && goIn ) {
                if (xSemaphoreTake(xSemaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
                  goIn = false;
                  updateMenu();
                  xSemaphoreGive(xSemaphore);
                  vTaskDelay(100 / portTICK_PERIOD_MS);
                }
              }
              if (menu == 7 && goIn ) {
                if (xSemaphoreTake(xSemaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
                  goIn = false;
                  updateMenu();
                  xSemaphoreGive(xSemaphore);
                  vTaskDelay(100 / portTICK_PERIOD_MS);
                }
              }
              if (menu == 8 && goIn ) {
                if (xSemaphoreTake(xSemaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
                  if (plusDefaults == 1) {
                    sendDataHardReset();
                    ResetDefaut();
                  }
                  else if (plusDefaults == 2) {
                    goIn = false;
                    updateMenu();
                    xSemaphoreGive(xSemaphore);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                  }
                }
              }
            }
          }
          else {
            break; // Botão foi liberado antes do tempo
          }
        }
        vTaskDelay(pdMS_TO_TICKS(100));

        if (statusDown == LOW && enter == false) {

          if (firstPression == false) {
            if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
              activateFeeder();                                                                   //  DESCOMENTARRRRRRRRRRRRRRRRRRRRRRRRRRR
              xSemaphoreGive(xSemaphore);
            }
          }
          else if (firstPression == true && goIn == false ) {
            if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
              menu++;
              updateMenu();
              Serial.println("firstPression == true && goIn == false ");

              xSemaphoreGive(xSemaphore);
            }
          }
          else if (menu == 1 && goIn == true) {
            if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {
              Serial.println("menu == 1 && goIn == true ");

              if (notplus == 1) {
                plus = 0;
                plusFeerder = 1;
              }
              if (notplus == 0) {
                plus = 1;
              }
              executeAction();
              //   vTaskDelay(500 / portTICK_PERIOD_MS);
              xSemaphoreGive(xSemaphore);
              vTaskDelay(200 / portTICK_PERIOD_MS);

            }
          }
          else if (menu == 2  && goIn ) {
            if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {
              if (notplus == 1) {
                plus = 0;
                plusIlumination = 1;
                plusIluminationMin = 5;
              }
              if (notplus == 0) {
                plus = 1;
              }
              executeAction();
              xSemaphoreGive(xSemaphore);
              vTaskDelay(200 / portTICK_PERIOD_MS);

            }
          }
          else if (menu == 3  && goIn ) {
            if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {
              if (notplus == 1) {
                plus = 0;
                plusTemp = 1;
              }
              if (notplus == 0) {
                plus = 1;
              }
              executeAction();
              xSemaphoreGive(xSemaphore);
              vTaskDelay(200 / portTICK_PERIOD_MS);

            }
          }

          else if (menu == 4  && goIn ) {
            if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {
              plus = 1;
              executeAction();
              xSemaphoreGive(xSemaphore);
              vTaskDelay(200 / portTICK_PERIOD_MS);

            }
          }
          else if (menu == 5  && goIn ) {
            if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {
              plus = 1;
              executeAction();
              xSemaphoreGive(xSemaphore);
              vTaskDelay(200 / portTICK_PERIOD_MS);

            }
          }
          else if (menu == 6 && goIn ) {
            if (xSemaphoreTake(xSemaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
              goIn = false;
              updateMenu();
              xSemaphoreGive(xSemaphore);
              vTaskDelay(100 / portTICK_PERIOD_MS);
            }
          }

          else if (menu == 7 && goIn ) {
            if (xSemaphoreTake(xSemaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
              goIn = false;
              updateMenu();
              xSemaphoreGive(xSemaphore);
              vTaskDelay(100 / portTICK_PERIOD_MS);
            }
          }
          else if (menu == 8  && goIn ) {
            if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {
              plus = 1;
              executeAction();
              xSemaphoreGive(xSemaphore);
            }
          }
        }
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
