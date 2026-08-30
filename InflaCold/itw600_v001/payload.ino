void inicialization() {

  EEPROM.write(memory.getAddrSetServer(), false);
  EEPROM.commit();

  Serial.println("                                                                        Verificando se ja foi inicializado");
  EEPROM.get(700, firstReset);  // Lê o valor da posição 0 da EEPROM
  Serial.println("''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''firstReset");  // Deve imprimir "42"
  Serial.println(firstReset);  // Deve imprimir "42"

  if (firstReset == -1 || firstReset == 1 ) {
    Serial.println("''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''FOi resetado padronizado as informaç~eos");  // Deve imprimir "42"

    interval = 300000;
    int addrTimer = memory.getAddrTimer();
    memory.saveMemoryUL(addrTimer, interval);

    isServerCreated = 0;
    Serial.println("Servidor Criou o Dispositivo");
    EEPROM.write(220, isServerCreated);
    EEPROM.commit();

    std::string mqttHostStr = std::string(mqttHost.c_str());
    std::string mqttPortStr = std::string(mqttPort.c_str());
    std::string mqttUserStr = std::string(mqttUser.c_str());
    std::string mqttPasswordStr = std::string(mqttPassword.c_str());

    sh.setMqttHost(mqttHostStr);
    sh.setMqttPort(mqttPortStr);
    sh.setMqttUser(mqttUserStr);
    sh.setMqttPassword(mqttPasswordStr);

  }
  firstReset = 0;
  EEPROM.put(700, firstReset);  // Salva o valor 42 na posição 0 da EEPROM
  EEPROM.commit();        // IMPORTANTE! Confirma a gravação na EEPROM

}

void serverCreated() {
  bool isFlagServerCreated = ph.getFlagCreated();
  if (isFlagServerCreated) {
    isServerCreated = isFlagServerCreated;
    Serial.println("Servidor Criou o Dispositivo");
    EEPROM.write(220, isServerCreated);
    EEPROM.commit();
    ph.setFlagCreated(0);

  }
}


// ============================ SETUP ============================
void setup() {
  Serial.begin(115200);
  sensors.begin();
  irrecv.enableIRIn();
  irsend.begin();
  EEPROM.begin(2048);  // <<< ADICIONE ISSO (ou mais, se quiser)

  sensors.requestTemperatures();
  T_filt = sensors.getTempCByIndex(0);
  T_prev = T_filt;

  AC_setpoint = targetTemp + 2;
  AC_setpoint = constrain(AC_setpoint, AC_MIN_TEMP, AC_MAX_TEMP);

  last_action_time = millis();

  loadConfig();
  printMainMenu();

    pinMode(resetButton, INPUT);





  Serial.setDebugOutput(true);
  vTaskDelay(pdMS_TO_TICKS(100));
  Serial.print("aadr ssid: " );
  Serial.println(memory.getAddrSSID());
  Serial.print("addr password: ");
  Serial.println(memory.getAddrPassword());
  PayloadHandle::initializeServerHandle(&sh);

  std::string siteUrlStr = std::string(siteUrl.c_str());
  sh.setSiteUrl(siteUrlStr);


  setReconnect = EEPROM.read(memory.getAddrReconnect());

  isServerCreated = EEPROM.read(220);

  std::string ssid_str = memory.readMemoryStr(memory.getAddrSSID());
  std::string passwordWifi_str = memory.readMemoryStr(memory.getAddrPassword());

  ssid = String(ssid_str.c_str());
  password = String(passwordWifi_str.c_str());

  Serial.print("ssid: ");
  Serial.println(ssid);
  Serial.print("password: ");
  Serial.println(password);
  Serial.print("addr password: ");
  Serial.println();


  serverInitialized = EEPROM.read(200);



  inicialization();


  xTaskCreatePinnedToCore(
    receveidPayload,
    "switchPayload",
    4096,
    NULL,
    9,
    &SwicthPayloadHandle,
    0
  );

  std::string mqttHostStr = std::string(mqttHost.c_str());
  std::string mqttPortStr = std::string(mqttPort.c_str());
  std::string mqttUserStr = std::string(mqttUser.c_str());
  std::string mqttPasswordStr = std::string(mqttPassword.c_str());

  sh.setMqttHost(mqttHostStr);
  sh.setMqttPort(mqttPortStr);
  sh.setMqttUser(mqttUserStr);
  sh.setMqttPassword(mqttPasswordStr);



  vTaskDelay(pdMS_TO_TICKS(1000));



  if (WiFi.status() == WL_CONNECTED && isServerCreated == true) {
    std::string mqttHostStr = std::string(mqttHost.c_str());
    std::string mqttPortStr = std::string(mqttPort.c_str());
    std::string mqttUserStr = std::string(mqttUser.c_str());
    std::string mqttPasswordStr = std::string(mqttPassword.c_str());

    sh.setMqttHost(mqttHostStr);
    sh.setMqttPort(mqttPortStr);
    sh.setMqttUser(mqttUserStr);
    sh.setMqttPassword(mqttPasswordStr);
    sh.connectToMQTTServer();
    Serial.println(sh.getMqttHost().c_str());
    Serial.println(sh.getMqttPort().c_str());
    Serial.println(sh.getMqttPassword().c_str());
    Serial.println(sh.getMqttUser().c_str());
  }
  sh.setMqttHost(mqttHostStr);


  if (WiFi.status() == WL_CONNECTED && isServerCreated) {

    buildTopic();
    sh.connectToMQTTServer();
    Serial.println(sh.getMqttHost().c_str());
    Serial.println(sh.getMqttPort().c_str());
    Serial.println(sh.getMqttPassword().c_str());
    Serial.println(sh.getMqttUser().c_str());
  }

  int addrSetTimer = memory.getAddrSetTimer();
  bool setTimer = memory.readMemoryBool(addrSetTimer);

  if (setTimer) {
    Serial.println("Alterado o time");
    int addrTimer = memory.getAddrTimer();
    interval = memory.readMemoryUL(addrTimer);
  }
  sh.setVersion(std::string(versionStr.c_str()));


  esp_netif_init();
  esp_event_loop_create_default();
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg_init = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg_init);
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_start();

  esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
  esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL);
  esp_event_handler_register(SC_EVENT, SC_EVENT_GOT_SSID_PSWD, &event_handler, NULL);

  esp_smartconfig_set_type(SC_TYPE_ESPTOUCH);
  esp_smartconfig_start(&cfg);

  //savetemp=idealTemp;
  Serial.println("\nAguardando SmartConfig...");
  contReset = EEPROM.read (598);
  Serial.println("Quantas vezes foi resetado:");
  Serial.println(contReset);


  if (setReconnect) {
    contReset = contReset + 1;
    EEPROM.write(598, contReset);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    EEPROM.commit();
  }

  if (contReset > 5) {

    setReconnect = false;
    addrReconnect = memory.getAddrReconnect();
    EEPROM.write(addrReconnect, setReconnect);
    EEPROM.commit();

    stopEspIDFStack();

    contReset = 0;
    EEPROM.write(598, contReset);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    EEPROM.commit();

    Serial.print("ptr addr: ");
    Serial.println(addrReconnect);
    Serial.print("setReconnect on button: ");
    Serial.println(EEPROM.read(addrReconnect));
    serverInitialized = false; // Marca como inicializado

    EEPROM.write(200, serverInitialized);
    EEPROM.commit();
    //wifiManager.resetSettings();
    ESP.restart();
  }
  
}


void loop() {
  // 1. Roda o termostato em segundo plano (se estiver ativado)
  runThermostat();

  // 2. Leitura dos Comandos do Usuário
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      if (buffer.length() == 0) continue;

      char cmd = toupper(buffer[0]); // Converte para maiúscula

      // ================= ESTADO 0: MENU PRINCIPAL =================
      if (state == 0) {
        switch (cmd) {
          // --- Auto Detectar ---
          case 'A':
            autoDetectAC(); // Já possui a lógica do LED Azul/Vermelho/Verde dentro dela
            printMainMenu();
            break;

          // --- Alternar Termostato ---
          case 'T':
            thermostatEnabled = !thermostatEnabled;
            Serial.print(">> Modo Termostato: ");
            Serial.println(thermostatEnabled ? "ATIVADO" : "DESATIVADO");
            // Se ativou, tenta salvar o estado na EEPROM também (opcional)
            saveConfig();
            printMainMenu();
            break;

          // --- Definir Temperatura Alvo (S23, S24...) ---
          case 'S':
            if (buffer.length() > 1) {
              int novaTemp = buffer.substring(1).toInt();

              // Validação básica de segurança (entre 16 e 30 graus)
              if (novaTemp >= 16 && novaTemp <= 30) {
                targetTemp = novaTemp;
                AC_setpoint = targetTemp + 2;
                AC_setpoint = constrain(AC_setpoint, AC_MIN_TEMP, AC_MAX_TEMP);
                Serial.print(">> Nova meta definida: ");
                Serial.println(targetTemp);

                // AQUI: Chama a função que grava na EEPROM e pisca o LED VERDE
                saveConfig();
              } else {
                Serial.println(">> Erro: Temperatura deve ser entre 16 e 30.");
              }
            }
            printMainMenu();
            break;

          // --- Outras Opções (Manter igual) ---
          case 'M': printDrivers(); state = 2; break;
          case 'C':
            if (activeAC) {
              state = 1;
              printControlMenu();
            }
            else Serial.println(">> Erro: Nenhum AC selecionado.");
            break;
          case 'L': printDrivers(); break;
        }
      }

      // ================= ESTADO 1: CONTROLE MANUAL DO AC =================
      else if (state == 1) {
        switch (cmd) {
          case 'L': activeAC->on(activeAC->obj); sendActive(); break;
          case 'D': activeAC->off(activeAC->obj); sendActive(); break;
          case '+': activeAC->tempUp(activeAC->obj); sendActive(); break;
          case '-': activeAC->tempDown(activeAC->obj); sendActive(); break;
          case 'C': activeAC->setCool(activeAC->obj); sendActive(); break;
          case 'R': activeAC->setDry(activeAC->obj); sendActive(); break;
          case 'F': activeAC->setFan(activeAC->obj); sendActive(); break;
          case 'B': sendActive(); break;
          case 'S': {
              int t = buffer.substring(1).toInt();
              activeAC->setTemp(activeAC->obj, t);
              sendActive();
            } break;
          case 'V': state = 0; printMainMenu(); break; // Voltar
        }
      }

      // ================= ESTADO 2: SELEÇÃO MANUAL DE DRIVER =================
      else if (state == 2) {
        int idx = buffer.toInt();
        if (idx >= 1 && idx <= DRIVER_COUNT) {
          activeAC = allDrivers[idx - 1];
          activeAC->begin(activeAC->obj);
          Serial.print(">> Driver selecionado manualmente: ");
          Serial.println(activeAC->name);

          // Salva na EEPROM e pisca LED VERDE
          saveConfig();
        }
        state = 0;
        printMainMenu();
      }

      buffer = ""; // Limpa o buffer para o próximo comando
    }
    else {
      buffer += c; // Vai montando a palavra (ex: "S", "S2", "S24")
    }
  }

/*
    if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {
      HTTPClient http;
      http.begin("http://clients3.google.com/generate_204");
      int httpCode = http.GET();
      xSemaphoreGive(xSemaphore);
    
  }
*/
  if (smartConfigDone ) {
    Serial.println("Esotu no loop");
    smartConfigDone = false;
    stopEspIDFStack();
    vTaskDelay(pdMS_TO_TICKS(100));
    connectWithArduinoWiFi();
  }

  serverSetupComplete();

  UBaseType_t  highWaterMark = uxTaskGetStackHighWaterMark(SwicthPayloadHandle);


  if (WiFi.status() == WL_CONNECTED) {
 //   if (serverCreated) {
//      sh.checkMQTTConnection();

//    }
    if (ph.getError()) {
      std::map<std::string, std::string> payloadMap;
      payloadMap["error"] = "1";
      std::string payload = sh.buildPayload(payloadMap);
      sh.sendPayload(payload, sh.getTopicData());
      ph.setError(false);
    }
    if (ph.getErrorDeserialization()) {
      std::map<std::string, std::string> payloadMap;
      payloadMap["error"] = "2";
      std::string payload = sh.buildPayload(payloadMap);
      sh.sendPayload(payload, sh.getTopicData());
      ph.setErrorDeserialization(false);
    }
    contReset = 0;
    EEPROM.write(598, contReset);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    EEPROM.commit();


    bool transmission = sh.getTransmission();
    if (transmission) {
      sh.setTransmission(false);
      sendData();
    }

    if (ConnectCont < 1) {
      ConnectCont = ConnectCont + 1;
      payloadInfo();
    }
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval)
    {
      Serial.println("Se passaram dois minutos");
      std::string tpc = sh.getTopicData();
      String tpcStr = String(tpc.c_str());
      previousMillis = currentMillis;
      sendData();
    }
    if (isCreatedWifi == true && WiFi.status() == WL_CONNECTED) {
      Serial.println("Enviando ID");
      sendDataIdcustomic();
      isCreatedWifi = false;
    }
    vTaskDelay(pdMS_TO_TICKS(300));
  }

  else {

    Serial.println("Esta no set reconnect ");
    std::string mqttHostStr = std::string(mqttHost.c_str());
    std::string mqttPortStr = std::string(mqttPort.c_str());
    std::string mqttUserStr = std::string(mqttUser.c_str());
    std::string mqttPasswordStr = std::string(mqttPassword.c_str());

    sh.setMqttHost(mqttHostStr);
    sh.setMqttPort(mqttPortStr);
    sh.setMqttUser(mqttUserStr);
    sh.setMqttPassword(mqttPasswordStr);
    if (setReconnect == true) {
      stopEspIDFStack();
    }
    wh.reconnectWifi(&memory);
    Serial.println(WiFi.localIP());
    buildTopic();

  }
  
}
