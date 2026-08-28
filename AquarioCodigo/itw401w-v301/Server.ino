
void receveidPayload(void *param) {
  while (1) {
    if (WiFi.status() == WL_CONNECTED) {
      serverCreated();
      sh.updateHandle();
      sh.payloadDeserialization();
      ph.payloadHandle();
      hardReset();
      wifiReset();

      if (isServerCreated) {
        signatureAdjust();
        getTransmisson();
        lastButtonIntensity = millis();
        checkAll();


        if (SignatureMqtt >= currentRtc) {
          feederRoutine();
          lampRoutine();
          lampImmediate();
          feederImmediate();
          tempAdjust();
          timeAjust();
          vTaskDelay(pdMS_TO_TICKS(100));  // Espera 500ms para não sobrecarregar a CPU

        }
      }
    }
    taskYIELD();  // Libera a CPU para outras tarefas
    vTaskDelay(pdMS_TO_TICKS(100));  // Espera 500ms para não sobrecarregar a CPU
  }
}

void serverSetupComplete() {

  if (WiFi.status() == WL_CONNECTED && !setReconnect) {

    Serial.println("WiFi conectado e setReconnect falso");

    // -------------------------------------------------
    // 1️⃣ RECONNECT (fica na EEPROM - MemoryManagment)
    // -------------------------------------------------

    setReconnect = true;
    memory.saveMemoryBool("reconnect", true);

    Serial.print("Reconnect salvo. Valor atual: ");
    Serial.println(memory.readMemoryBool("reconnect", true));

    // -------------------------------------------------
    // 2️⃣ SERVER INITIALIZED (vai para Preferences)
    // -------------------------------------------------

    serverInitialized = true;
    prefs.putBool("serverInit", serverInitialized);

    // -------------------------------------------------
    // 3️⃣ SALVAR SSID E SENHA (fica na EEPROM)
    // -------------------------------------------------

    memory.saveMemoryStr("password",
                         std::string(WiFi.psk().c_str()));

    memory.saveMemoryStr("ssid",
                         std::string(WiFi.SSID().c_str()));

    // -------------------------------------------------
    // 4️⃣ RESETAR CONTADOR (vai para Preferences)
    // -------------------------------------------------

    contReset = 0;
    prefs.putInt("contReset", contReset);

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
  sendDataCooler();
  vTaskDelay(500 / portTICK_PERIOD_MS);
  sendDataHeater();
  vTaskDelay(500 / portTICK_PERIOD_MS);
  sendDataL1();
  vTaskDelay(500 / portTICK_PERIOD_MS);
  sendDataL2();
  vTaskDelay(500 / portTICK_PERIOD_MS);
  sendDataL3();
  vTaskDelay(500 / portTICK_PERIOD_MS);
  sendDataFeeder();
  vTaskDelay(500 / portTICK_PERIOD_MS);
  //sendDataTempControllConfig();
  //vTaskDelay(500 / portTICK_PERIOD_MS);
  sendDataTemp();
  vTaskDelay(500 / portTICK_PERIOD_MS);
  sendDatasignature();
  vTaskDelay(500 / portTICK_PERIOD_MS);

  /*
    if (statusIlum == "  ATIVADO") {
      sendLampRoutine(illuminationStartHour, illuminationStartMinute, illuminationEndHour, illuminationEndMinute);
    }
    else if (routineIlu) {
      for (int i = 0; i < routineCountIlumi; i++) {
        LampRoutine routine = lampRoutines[i];

        sendRoutineIlu(
          routine.hour,
          routine.minutes,
          routine.intensityL1,
          routine.intensityL2,
          routine.intensityL3
        );
      }
    }
    if (feederMode == "  ATIVADO") {

      sendfeederRoutine(1, feedingHour, feedingMinute);

    }

    else if (routineFeeder) {
      for (int i = 0; i < routineCountFeeder; i++) {
        FeederRoutine routine = feederRoutines[i];

        sendfeederRoutine(
          routine.spins,
          routine.hour,
          routine.minutes
        );
      }
    }
  */
}

void sendfeederRoutineConfig(int spinsProg, int hourProg, int minProg) {
  checkError();
  String timeProgramFeeder = (hourProg < 10 ? "0" : "") + String(hourProg) + ":" +  (minProg < 10 ? "0" : "") + String(minProg);

  std::map<std::string, std::string> payloadMap;

  payloadMap["feeder"] =  "config";
  payloadMap["spins"] =  std::to_string (spinsProg);
  payloadMap["time"] = std::string(timeProgramFeeder.c_str());

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicConfig());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}


void sendfeederRoutine(int spinsProg, int hourProg, int minProg) {
  checkError();
  String timeProgramFeeder = (hourProg < 10 ? "0" : "") + String(hourProg) + ":" +  (minProg < 10 ? "0" : "") + String(minProg);

  std::map<std::string, std::string> payloadMap;

  payloadMap["spins"] =  std::to_string (spinsProg);
  payloadMap["time"] = std::string(timeProgramFeeder.c_str());

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicConfig());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}
void sendfeederRoutineDisplay(int spinsProg, int hourProg, int minProg) {
  checkError();
  String timeProgramFeeder = (hourProg < 10 ? "0" : "") + String(hourProg) + ":" +  (minProg < 10 ? "0" : "") + String(minProg);

  std::map<std::string, std::string> payloadMap;
  payloadMap["spins"] =  std::to_string (spinsProg);
  payloadMap["time"] = std::string(timeProgramFeeder.c_str());

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicConfig());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}

void sendRoutineIlu(int hourProgOn, int minProgOn, int int1, int int2, int int3) {
  checkError();

  String timeProgramLightOn = (hourProgOn < 10 ? "0" : "") + String(hourProgOn) + ":" +  (minProgOn < 10 ? "0" : "") + String(minProgOn);
  std::map<std::string, std::string> payloadMap;
  payloadMap["light"] = std::string("config");
  payloadMap["time"] = std::string(timeProgramLightOn.c_str());

  payloadMap["l1"] =  std::to_string(int1);
  payloadMap["l2"] =  std::to_string(int2);
  payloadMap["l3"] =  std::to_string(int3);
  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicConfig());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}


void sendDataHeater() {
  checkError();

  std::map<std::string, std::string> payloadMap;
  payloadMap["heater"] =  std::to_string (controllError[5]);
  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicData());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);

}



void sendDataCooler() {
  checkError();

  std::map<std::string, std::string> payloadMap;
  payloadMap["cooler"] =  std::to_string (controllError[3]);

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicData());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}

void sendDataL1() {
  checkError();

  std::map<std::string, std::string> payloadMap;
  payloadMap["l1"] =  std::to_string (controllError[0]);
  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicData());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}


void sendDataL2() {
  checkError();

  std::map<std::string, std::string> payloadMap;
  payloadMap["l2"] =  std::to_string (controllError[1]);

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicData());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);

}


void sendDataL3() {
  checkError();

  std::map<std::string, std::string> payloadMap;
  payloadMap["l3"] =  std::to_string (controllError[2]);

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicData());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);

}

void sendDataL1L2L3() {
  checkError();

  std::map<std::string, std::string> payloadMap;
  payloadMap["confirm"] = "ok";
  payloadMap["l1"] =  std::to_string ( controllError[0]);
  payloadMap["l2"] =  std::to_string ( controllError[1]);
  payloadMap["l3"] =  std::to_string ( controllError[2]);

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicInfo());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);

}


void sendDataHardResetConfirm() {
  checkError();

  std::map<std::string, std::string> payloadMap;
  payloadMap["confirm"] = "ok";
  payloadMap["hardReset"] =  "true";

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicInfo());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}



void sendDataWifiResetConfirm() {
  checkError();

  std::map<std::string, std::string> payloadMap;
  payloadMap["confirm"] = "ok";
  payloadMap["wifiReset"] =  "true";

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicInfo());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}

void sendDataHardReset() {
  checkError();

  std::map<std::string, std::string> payloadMap;
  payloadMap["hardReset"] =  "true";


  std::string payload = sh.buildPayload(payloadMap);
  //  if (isServerCreated) {
  sh.sendPayload(payload, sh.getTopicConfig());
  // }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}

void sendDataWifiReset() {
  checkError();

  std::map<std::string, std::string> payloadMap;
  payloadMap["wifiReset"] =  "true";


  std::string payload = sh.buildPayload(payloadMap);
  //  if (isServerCreated) {
  sh.sendPayload(payload, sh.getTopicConfig());
  // }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}

void sendDataFeeder() {
  checkError();

  std::map<std::string, std::string> payloadMap;
  payloadMap["feeder"] =  std::to_string (controllError[4]);

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicData());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);

}

void sendDataFeederConfirm() {
  checkError();

  std::map<std::string, std::string> payloadMap;
  payloadMap["feeder"] =  std::to_string (controllError[4]);
  payloadMap["confirm"] = "ok";

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicInfo());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}



void sendDataTempControllConfirm() {
  checkError();
  std::string idealStr;
  std::ostringstream idealStream;
  idealStream << std::fixed << std::setprecision(1) << range;
  idealStr = idealStream.str();
  vTaskDelay(pdMS_TO_TICKS(100));

  std::map<std::string, std::string> payloadMap;
  payloadMap["ideal"] =  std::to_string (savetemp);
  payloadMap["range"] =  idealStr;
  payloadMap["confirm"] = "ok";

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicInfo());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}

void sendDataTempControll() {
  checkError();
  std::string idealStr;
  std::ostringstream idealStream;
  idealStream << std::fixed << std::setprecision(1) << range;
  idealStr = idealStream.str();
  vTaskDelay(pdMS_TO_TICKS(100));

  std::map<std::string, std::string> payloadMap;
  payloadMap["ideal"] =  std::to_string (savetemp);
  payloadMap["range"] =  idealStr;

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicData());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}



void sendDataTempControllConfig() {
  checkError();
  std::string idealStr;
  std::ostringstream idealStream;
  idealStream << std::fixed << std::setprecision(1) << range;
  idealStr = idealStream.str();
  vTaskDelay(pdMS_TO_TICKS(100));

  std::map<std::string, std::string> payloadMap;
  payloadMap["ideal"] =  std::to_string (savetemp);
  payloadMap["range"] =  idealStr;

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicConfig());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}



void sendDataTemp() {
  checkError();
  std::string tempStr;
  std::ostringstream tempStream;
  tempStream << std::fixed << std::setprecision(1) << temp;
  tempStr = tempStream.str();
  displayTemp = temp;
  std::map<std::string, std::string> payloadMap;
  payloadMap["temp"] =  tempStr;

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicData());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);

}


void sendDataLampRoutine() {
  checkError();
  std::string tempStr;
  std::ostringstream tempStream;
  tempStream << std::fixed << std::setprecision(1) << temp;
  tempStr = tempStream.str();

  std::map<std::string, std::string> payloadMap;
  payloadMap["l1"] =  std::to_string (timeLamp1);
  payloadMap["l2"] =  std::to_string (timeLamp2);
  payloadMap["l3"] =  std::to_string (timeLamp3);
  payloadMap["time"] = timeLamp.c_str();
  payloadMap["confirm"] = "ok";

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicInfo());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}




void sendLampRoutine(int hourProgOn, int minProgOn, int hourProgOff, int minProgOff) {
  checkError();

  String timeProgramLightOn = (hourProgOn < 10 ? "0" : "") + String(hourProgOn) + ":" +  (minProgOn < 10 ? "0" : "") + String(minProgOn);
  String timeProgramLightOff = (hourProgOff < 10 ? "0" : "") + String(hourProgOff) + ":" +  (minProgOff < 10 ? "0" : "") + String(minProgOff);
  std::map<std::string, std::string> payloadMap;
  payloadMap["light"] = std::string("config");
  payloadMap["start"] = std::string(timeProgramLightOn.c_str());
  payloadMap["end"] = std::string(timeProgramLightOff.c_str());

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicConfig());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}






void sendDataRTC() {
  checkError();
  std::string tempStr;
  std::ostringstream tempStream;
  tempStream << std::fixed << std::setprecision(1) << temp;
  tempStr = tempStream.str();

  std::map<std::string, std::string> payloadMap;
  payloadMap["rtc"] =  std::to_string (rtcMqtt);
  payloadMap["confirm"] = "ok";

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicInfo());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}



void sendDataConfirmRoutineLight() {


  std::map<std::string, std::string> payloadMap;
  payloadMap["light"] = "routine";
  payloadMap["delete"] = "true";
  payloadMap["confirm"] = "ok";

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicInfo());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}


void sendDataConfirmRoutineFeeder() {


  std::map<std::string, std::string> payloadMap;
  payloadMap["feeder"] = "routine";
  payloadMap["delete"] = "true";
  payloadMap["confirm"] = "ok";

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicInfo());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}

void sendDatasignatureConfirm() {
  checkError();


  std::map<std::string, std::string> payloadMap;
  payloadMap["subscription"] =  std::to_string (SignatureMqtt);
  payloadMap["confirm"] = "ok";

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicData());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}

void sendDatasignature() {
  checkError();


  std::map<std::string, std::string> payloadMap;
  payloadMap["subscription"] =  std::to_string (SignatureMqtt);

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicData());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}

void sendDatafeederRoutine() {
  checkError();
  std::string tempStr;
  std::ostringstream tempStream;
  tempStream << std::fixed << std::setprecision(1) << temp;
  tempStr = tempStream.str();

  std::map<std::string, std::string> payloadMap;
  payloadMap["spins"] =  std::to_string (timefeederSpins);
  payloadMap["time"] = timeFeederMqtt.c_str();
  payloadMap["confirm"] = "ok";

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicData());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}

void sendDataError2() {


  std::map<std::string, std::string> payloadMap;
  payloadMap["error"] =  std::to_string (2);

  std::string payload = sh.buildPayload(payloadMap);
  if (isServerCreated) {
    sh.sendPayload(payload, sh.getTopicData());
  }
  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);
}

void sendDataIdcustomic() {
  //checkError();

  std::map<std::string, std::string> payloadMap;
  payloadMap["customerName"] =  std::string(customerName.c_str());

  std::string payload = sh.buildPayload(payloadMap);
  sh.sendPayload(payload, sh.getTopicInfo());

  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);

}

void sendDeleteIlumination() {
  //checkError();

  std::map<std::string, std::string> payloadMap;
  payloadMap["light"] =  "routine";
  payloadMap["delete"] =  "true";

  std::string payload = sh.buildPayload(payloadMap);
  sh.sendPayload(payload, sh.getTopicConfig());

  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);

}


void sendDeleteFeeder() {
  //checkError();

  std::map<std::string, std::string> payloadMap;
  payloadMap["feeder"] =  "routine";
  payloadMap["delete"] =  "true";

  std::string payload = sh.buildPayload(payloadMap);
  sh.sendPayload(payload, sh.getTopicConfig());

  String payloadStr = String(payload.c_str());
  Serial.print("Payload sent: ");
  Serial.println(payloadStr);

}

const char* serverUrl = "https://iotew.com.br/updates/itw405w_v100.ino.esp32";
void testarRequisicao() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);

    Serial.print("Enviando requisição GET para: ");
    Serial.println(serverUrl);

    int httpCode = http.GET(); // Faz a requisição

    Serial.print("Código de resposta HTTP: ");
    Serial.println(httpCode);

    if (httpCode > 0) {
      String payload = http.getString(); // Lê a resposta do servidor
      Serial.println("Resposta do servidor:");
      //Serial.println(payload);
    } else {
      Serial.println("Falha na requisição!");
    }

    http.end(); // Finaliza a conexão
  } else {
    Serial.println("WiFi desconectado!");
  }
}
