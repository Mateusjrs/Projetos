void receveidPayload(void *param) {
  while (1) {
    if (WiFi.status() == WL_CONNECTED) {
      serverCreated();
      sh.updateHandle();
      sh.payloadDeserialization();
      ph.payloadHandle();

      if (isServerCreated) {
        getTransmisson();        
        hardReset();
        vTaskDelay(pdMS_TO_TICKS(100));  // Espera 500ms para não sobrecarregar a CPU
      }
    }
    taskYIELD();  // Libera a CPU para outras tarefas
    vTaskDelay(pdMS_TO_TICKS(100));  // Espera 500ms para não sobrecarregar a CPU
  }
}


void getTransmisson() {
  bool transmission = sh.getTransmission();

  if (transmission) {
    Serial.println("Mqtt pediu para enviar.");
    sendData();
    sh.setTransmission(0);
  }
}


void hardReset() {
  bool flagHardReset = ph.getFlagHardReset();
  if (flagHardReset) {
//    sendDataHardResetConfirm();
    //vTaskDelay(pdMS_TO_TICKS(5000));
    ResetDefaut();
    ph.setFlagHardReset(0);
  }

}

void sendDataIdcustomic() {
  //checkError();

  std::map<std::string, std::string> payloadMap;
  payloadMap["customerName"] =  std::string(customerName.c_str());

  std::string payload = sh.buildPayload(payloadMap);
  sh.sendPayload(payload, sh.getTopicConfig());

  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);

}

void ResetDefaut() {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0xFF);  // Apaga toda a EEPROM (0xFF = valor padrão)
  }
  EEPROM.commit();

//  wifiManager.resetSettings();
  setReconnect = false;
  addrReconnect = memory.getAddrReconnect();
  EEPROM.write(addrReconnect, setReconnect);
  EEPROM.commit();

  Serial.print("ptr addr: ");
  Serial.println(addrReconnect);
  Serial.print("setReconnect on button: ");
  Serial.println(EEPROM.read(addrReconnect));
  serverInitialized = false; // Marca como inicializado
  EEPROM.write(200, serverInitialized);
  EEPROM.commit();
  firstReset = 1;
  EEPROM.put(700, firstReset);  // Salva o valor 42 na posição 0 da EEPROM
  EEPROM.commit();        // IMPORTANTE! Confirma a gravação na EEPROM

  Serial.println("EEPROM apagada!");
  ESP.restart();
}


void serverSetupComplete   () {
  if (WiFi.status() == WL_CONNECTED && !(setReconnect)) {
    Serial.println("WiFi.status() == WL_CONNECTED && !(setReconnect)");
    setReconnect = true;
    addrReconnect = memory.getAddrReconnect();
    EEPROM.write(addrReconnect, true);
    EEPROM.commit();
    Serial.print("this is the reconnect: ");
    Serial.println(EEPROM.read(addrReconnect));

    serverInitialized = true; // Marca como inicializado
    EEPROM.write(200, serverInitialized);
    EEPROM.commit();
    vTaskDelay(pdMS_TO_TICKS(100));
    memory.saveMemoryStr(memory.getAddrPassword(), std::string(WiFi.psk().c_str()));
    EEPROM.commit();
    vTaskDelay(pdMS_TO_TICKS(100));
    memory.saveMemoryStr(memory.getAddrSSID(), std::string (WiFi.SSID().c_str()));
    vTaskDelay(pdMS_TO_TICKS(100));

    contReset = 0;
    EEPROM.write(598, contReset);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    EEPROM.commit();
  }
}

void payloadInfo() {
  std::map<std::string, std::string> payloadMap;
  payloadMap["version"] = versionStr.c_str();
  payloadMap["ssid"] = (WiFi.SSID().c_str());
  payloadMap["server"] = "connected";
  payloadMap["status"] = "OK";

  std::string payload = sh.buildPayload(payloadMap);
  sh.sendPayload(payload, sh.getTopicInfo());
}


void buildTopic()
{
  String baseVersion = version.substring(0, dashIndex - 1);
  String typeConnection = version.substring(dashIndex - 1, dashIndex);
  String result = baseVersion + "/" + typeConnection + "/";

  std::string topic = std::string(result.c_str());
  std::string topicConfig = topic;
  std::string topicControl = topic;
  std::string topicInfo = topic;

  MacAddressString = WiFi.macAddress();
  MacAddressString.replace(":", "");
  MacAddressString.remove(0, 5);

  std::string MacAdress = std::string(MacAddressString.c_str());
  topic += MacAdress;
  topicConfig += MacAdress;
  topicControl += MacAdress;
  topicInfo += MacAdress;
  topic += "/data";
  topicConfig += "/config";
  topicInfo += "/info";
  topicControl += "/control";

  sh.setTopicData(topic);
  sh.setTopicConfig(topicConfig);
  sh.setTopicControl(topicControl);
  sh.setTopicInfo(topicInfo);
}

void sendData() {

}
