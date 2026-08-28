


void displayFeeder() {
  if (feederMode == "ATIVADO") {
    controllFood(feedingHour, feedingMinute, 1);
  }
}
/*
  void saveRoutinefeeder() {
  Serial.println("Salvando rotinas na EEPROM...");
  for (int i = 0; i < MAX_FEEDER_ROUTINES; i++) {
    EEPROM.put(i * sizeof(FeederRoutine) + 950, feederRoutines[i]);
  }
  EEPROM.commit();
  }
*/


void saveRoutinefeeder() {

  Serial.println("Salvando rotinas na EEPROM...");

  const int startAddress = EEPROM_FEEDER_ROUTINES;
  const int routineSize  = sizeof(FeederRoutine);

  int lastAddress = startAddress +
                    (MAX_FEEDER_ROUTINES * routineSize) - 1;

  Serial.print("Tamanho da struct: ");
  Serial.println(routineSize);

  Serial.print("Endereco inicial: ");
  Serial.println(startAddress);

  Serial.print("Endereco final: ");
  Serial.println(lastAddress);

  for (int i = 0; i < MAX_FEEDER_ROUTINES; i++) {

    int currentAddress = startAddress + (i * routineSize);

    Serial.print("Salvando rotina ");
    Serial.print(i);
    Serial.print(" no endereco: ");
    Serial.println(currentAddress);

    EEPROM.put(currentAddress, feederRoutines[i]);
  }

  EEPROM.commit();
}

void backupRoutinefeeder() {

  Serial.println("Recuperando rotinas da EEPROM...");

  const int startAddress = EEPROM_FEEDER_ROUTINES;
  const int routineSize  = sizeof(FeederRoutine);

  routineCountFeeder = 0;          // 🔹 Zera antes de carregar
  currentRoutineIndexFeeder = 0;   // 🔹 Reinicia índice
  int savedCount = prefs.getInt("indexFeeder", 0);

  routineCountFeeder = 0;

  for (int i = 0; i < savedCount && i < MAX_FEEDER_ROUTINES; i++) {

    FeederRoutine rotina;
    int currentAddress = startAddress + (i * routineSize);

    EEPROM.get(currentAddress, rotina);

    if (rotina.spins > 0 &&
        rotina.hour >= 0 && rotina.hour < 24 &&
        rotina.minutes >= 0 && rotina.minutes < 60) {

      feederRoutines[routineCountFeeder] = rotina;
      routineCountFeeder++;
    }
  }

  // 🔹 Ajusta índice circular corretamente
  currentRoutineIndexFeeder = routineCountFeeder % MAX_FEEDER_ROUTINES;

  Serial.print("Total de rotinas carregadas: ");
  Serial.println(routineCountFeeder);
  showRoutinelight();

}

void showRoutinefeeder() {
  // Serial.println("Rotinas de alimentação:");
  for (int i = 0; i < routineCountFeeder; i++) {
    FeederRoutine routine = feederRoutines[i];
    Serial.print("Rotina ");
    Serial.print(i + 1);
    Serial.print(": Hora: ");
    Serial.print(routine.hour);
    Serial.print(", Minutos: ");
    Serial.print(routine.minutes);
    Serial.print(", Spins: ");
    Serial.println(routine.spins);
  }
}

unsigned long lastPayloadFeeder = 0; // Variável para armazenar o tempo do último payload


void applyRoutinefeeder() {
  for (int i = 0; i < routineCountFeeder; i++) {
    FeederRoutine routine = feederRoutines[i];
    time_t now = myRTC.get();  // Obtém o tempo atual do RTC
    tmElements_t tm;
    breakTime(now, tm);  // Decompõe o tempo em componentes

    if (tm.Hour == routine.hour && tm.Minute == routine.minutes) {
      controllFood(routine.hour, routine.minutes, routine.spins);
    }
  }
}

void feederRoutine() {
  // 🛡️ TRAVA DE REENTRÂNCIA (MUTEX)
  static bool isProcessingFeeder = false;
  if (isProcessingFeeder) return;
  isProcessingFeeder = true;

  feederRoutineMqtt = ph.getFlagFeederRoutine();
  bool feederRoutineMqttTime = ph.getFlagFeederRoutineTime();
  bool feederDelete = ph.getFlagDeleteRoutineFeeder();

  // Variáveis para controlar a "tempestade" de payloads e salvar apenas no final
  static unsigned long lastStormTimeFeeder = 0;
  static bool pendingEEPROMSaveFeeder = false;

  // ==========================================
  // 1. LÓGICA DE DELETAR ROTINAS
  // ==========================================
  if (feederDelete) {
    sendDataConfirmRoutineFeeder();
    next = static_cast<unsigned long long>(0);

    memset(feederRoutines, 0, sizeof(feederRoutines));
    routineCountFeeder = 0;
    currentRoutineIndexFeeder = 0;

    for (int i = 0; i < MAX_FEEDER_ROUTINES; i++) {
      FeederRoutine emptyRoutine = {0, 0, 0};
      int currentAddress = EEPROM_FEEDER_ROUTINES + (i * sizeof(FeederRoutine));
      EEPROM.put(currentAddress, emptyRoutine);
    }
    EEPROM.commit(); // É seguro dar commit aqui, pois é uma ação isolada

    Serial.println("Dados do struct e EEPROM do Feeder limpos.");
    ph.setFlagDeleteRoutineFeeder(0);
  }

  prefs.putBool("routineFeeder", routineFeeder);

  // ==========================================
  // 2. RECEBENDO DADOS RAPIDAMENTE (RAM)
  // ==========================================
  if (feederRoutineMqtt && feederRoutineMqttTime) {

    unsigned long currentTimeFeeder = millis();

    // Se faz mais de 10 segundos desde o último pacote, prepara a RAM para um novo lote
    if (currentTimeFeeder - lastPayloadFeeder > 10000 || lastPayloadFeeder == 0 ) {
      memset(feederRoutines, 0, sizeof(feederRoutines));
      routineCountFeeder = 0;
      currentRoutineIndexFeeder = 0;

      // NÃO FAZEMOS EEPROM.commit() AQUI!
      Serial.println("Limpando RAM do Feeder para novo lote...");
    }

    lastPayloadFeeder = currentTimeFeeder;
    lastStormTimeFeeder = currentTimeFeeder; // Marca o tempo do último pacote recebido

    timefeederSpins = ph.feederSpins[0];

    if (timefeederSpins > 0) {
      timeFeederMqtt = ph.getTime();
      int colonIndex = timeFeederMqtt.indexOf(':');
      int hourFeeder = timeFeederMqtt.substring(0, colonIndex).toInt();
      int minutesFeeder = timeFeederMqtt.substring(colonIndex + 1).toInt();

      sendDatafeederRoutine();
      FeederRoutine newRoutine = {timefeederSpins, hourFeeder, minutesFeeder};

      feederRoutines[currentRoutineIndexFeeder] = newRoutine;
      Serial.printf("Indice Feeder salvo na RAM: %d\n", currentRoutineIndexFeeder);

      currentRoutineIndexFeeder = (currentRoutineIndexFeeder + 1) % 5;
      if (routineCountFeeder < 5) {
        routineCountFeeder++;
      }

      // Sinaliza que precisamos salvar na EEPROM quando a tempestade passar
      pendingEEPROMSaveFeeder = true;
    }

    if (routineCountFeeder != 0) {
      routineFeeder = true;
      prefs.putBool("routineFeeder", routineFeeder);
      feederMode = "DESATIVADO";
      feederOn = 0;
      prefs.putBool("feederOn", feederOn);

    }

    // ==========================================
    // 3. LIBERAÇÃO DA CATRACA PRO MQTT
    // ==========================================
    ph.feederSpins[0] = -1;
    ph.setFlagFeederRoutine(0);
    ph.setFlagFeederRoutineTime(0);
  }

  // ==========================================
  // 4. SALVAR NA EEPROM (APÓS A TEMPESTADE)
  // ==========================================
  // Se temos dados pendentes e já passou 1 segundo desde o último payload (fim da rajada)
  if (pendingEEPROMSaveFeeder && (millis() - lastStormTimeFeeder > 1000)) {
    Serial.println("Rajada MQTT do Feeder finalizada! Salvando na EEPROM agora...");

    saveRoutinefeeder(); // Salva tudo de uma vez sem travar o ESP32!
    prefs.putInt("indexFeeder", routineCountFeeder);
    pendingEEPROMSaveFeeder = false; // Desativa a flag
  }

  // ==========================================
  // 5. APLICAÇÃO DA ROTINA
  // ==========================================
  if (routineFeeder) {
    applyRoutinefeeder();
  }

  checkNextLastFeeding();

  // 🔓 Tira o cadeado para a próxima execução da função
  isProcessingFeeder = false;
}
void checkNextLastFeeding() {
  time_t now = myRTC.get();  // Obtém o tempo atual do RTC
  tmElements_t tmNow;
  breakTime(now, tmNow);  // Decompõe o tempo atual

  time_t lastTime = 0;  // Inicializa com 0 (nenhuma alimentação anterior)
  time_t nextTime = 0;  // Inicializa com 0 (nenhuma alimentação futura)

  for (int i = 0; i < routineCountFeeder; i++) {
    FeederRoutine routine = feederRoutines[i];
    time_t routineTime = getTimestamp(routine.hour, routine.minutes);
    if (routineTime <= now) {
      if (lastTime == 0 || routineTime > lastTime) {
        lastTime = routineTime;
      }
    } else {
      // Se o horário da rotina ainda não chegou, atualiza o nextTime
      if (nextTime == 0 || routineTime < nextTime) {
        nextTime = routineTime;
      }
    }
  }
  // Se não houver próxima alimentação para o dia atual, programamos para o dia seguinte
  if (nextTime == 0) {
    FeederRoutine firstRoutine = feederRoutines[0];
    nextTime = getTimestamp(firstRoutine.hour, firstRoutine.minutes, 1);  // Programado para o próximo dia
  }
  // Se não houver última alimentação (i.e., estamos antes da primeira do dia), pegamos a última do dia anterior
  if (lastTime == 0) {
    FeederRoutine lastRoutine = feederRoutines[routineCountFeeder - 1];
    lastTime = getTimestamp(lastRoutine.hour, lastRoutine.minutes, -1);  // Última alimentação do dia anterior
  }
  if (routineCountFeeder == 0) {
    //Serial.println("Nenhuma rotina de alimentação armazenada.");
    lastTime = 0;
    nextTime = 0;
    return;  // Sai da função se não houver rotinas
  }
  // Serial.print("Last feeding mqtt: ");
  last =  static_cast<unsigned long long>(lastTime);  // Converte para long long
  //Serial.println(last);
  //Serial.print("Next feeding mqtt: ");
  next =  static_cast<unsigned long long>(nextTime);  // Converte para long long
  //Serial.println(next);
}

void applyLastRoutine() {
  time_t now = myRTC.get();  // Obtém o tempo atual do RTC
  tmElements_t tm;
  breakTime(now, tm);  // Decompõe o tempo em componentes
  int currentMinutes = tm.Hour * 60 + tm.Minute;  // Hora atual em minutos
  LampRoutine* lastRoutine = nullptr;
  LampRoutine* currentRoutine = nullptr;
  // Itera sobre as rotinas para encontrar a última e a atual
  for (int i = 0; i < routineCountIlumi; i++) {
    LampRoutine* routine = &lampRoutines[i];
    int routineMinutes = routine->hour * 60 + routine->minutes;
    // Se o tempo da rotina é exatamente agora, aplicar imediatamente
    // Serial.println("Rotina exatamente agora");
    if (routineMinutes == currentMinutes) {
      currentRoutine = routine;  // Armazena a rotina atual
      break;  // Não precisa continuar a busca
    } else if (routineMinutes < currentMinutes) {


      // Se a rotina é anterior ao tempo atual, armazena como última válida
      if (lastRoutine == nullptr || routineMinutes > (lastRoutine->hour * 60 + lastRoutine->minutes)) {
        lastRoutine = routine;
      }
    }
  }
  if (currentRoutine != nullptr) {
    activateLightMqtt(currentRoutine->intensityL1, currentRoutine->intensityL2, currentRoutine->intensityL3, currentRoutine->hour, currentRoutine->minutes);
    mediaFeeder = (currentRoutine->intensityL1 + currentRoutine->intensityL2 + currentRoutine->intensityL3) / 3;

  } else if (lastRoutine != nullptr) {
    // Se não houver rotina exata para agora, aplica a última
    activateLightMqtt(lastRoutine->intensityL1, lastRoutine->intensityL2, lastRoutine->intensityL3, lastRoutine->hour, lastRoutine->minutes);
    mediaFeeder = (lastRoutine->intensityL1 + lastRoutine->intensityL2 + lastRoutine->intensityL3) / 3;
  } else {
    // Caso não haja uma rotina anterior, apaga as luzes

  }
  if (routineCountIlumi != 0) {
  }
  if (routineCountIlumi == 0) {

  }
}

// Função para salvar as rotinas de iluminação na EEPROM
void saveRoutinelight() {

  Serial.println("Salvando rotinas de iluminação na EEPROM...");

  const int startAddress = EEPROM_LAMP_ROUTINES;
  const int routineSize  = sizeof(LampRoutine);

  for (int i = 0; i < MAX_LAMP_ROUTINES; i++) {

    int currentAddress = startAddress + (i * routineSize);

    EEPROM.put(currentAddress, lampRoutines[i]);

    Serial.print("Salvando rotina ");
    Serial.print(i);
    Serial.print(" no endereco: ");
    Serial.println(currentAddress);
  }

  EEPROM.commit();
}

void backupRoutinelight() {

  Serial.println("Recuperando rotinas de iluminação da EEPROM...");

  const int startAddress = EEPROM_LAMP_ROUTINES;
  const int routineSize  = sizeof(LampRoutine);

  routineCountIlumi = 0;           // 🔹 Zera antes de carregar
  currentRoutineIndexLamp = 0;     // 🔹 Reinicia índice circular
  routineCountIlumi = prefs.getInt("indexLight", -1);
  int savedCount = prefs.getInt("indexLight", 0);
  routineCountIlumi = 0;

  for (int i = 0; i < savedCount && i < MAX_LAMP_ROUTINES; i++) {

    LampRoutine rotina;
    int currentAddress = startAddress + (i * routineSize);

    EEPROM.get(currentAddress, rotina);

    if (rotina.hour >= 0 && rotina.hour < 24 &&
        rotina.minutes >= 0 && rotina.minutes < 60) {

      lampRoutines[routineCountIlumi] = rotina;
      routineCountIlumi++;
    }
  }

  // Ajuste do índice circular
  currentRoutineIndexLamp = routineCountIlumi % MAX_LAMP_ROUTINES;

  Serial.print("Total de rotinas carregadas: ");
  Serial.println(routineCountIlumi);
}

// Função que exibe as rotinas de iluminação atuais

unsigned long lastPayloadLamp = 0; // Variável para armazenar o tempo do último payload


void hardReset() {
  bool flagHardReset = ph.getFlagHardReset();
  if (flagHardReset) {
    sendDataHardResetConfirm();
    //vTaskDelay(pdMS_TO_TICKS(5000));
    ResetDefaut();
    ph.setFlagHardReset(0);
  }

}


void wifiReset() {
  bool flagWifiReset = ph.getFlagWifiReset();
  if (flagWifiReset) {
    sendDataWifiResetConfirm();
    //vTaskDelay(pdMS_TO_TICKS(5000));
    ph.setFlagHardReset(0);

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

}

void lampRoutine() {
  // Trava de reentrância (Mutex)
  static bool isProcessingLamp = false;
  if (isProcessingLamp) return;
  isProcessingLamp = true;

  lampRoutineMqtt = ph.getFlagLampRoutine();
  bool lampRoutineMqttTime = ph.getFlagLampRoutineTime();
  bool deleteLamp =  ph.getFlagDeleteRoutineLamp();

  // Variável estática para marcar quando a "tempestade" de payloads começou
  static unsigned long lastStormTime = 0;
  // Flag para avisar que temos dados na RAM pendentes de irem para a EEPROM
  static bool pendingEEPROMSave = false;

  // ==========================================
  // 1. LÓGICA DE DELETAR ROTINAS
  // ==========================================
  if (deleteLamp) {
    sendDataConfirmRoutineLight();
    updateLightIntensity(light1, 0);
    updateLightIntensity(light2, 0);
    updateLightIntensity(light3, 0);

    memset(lampRoutines, 0, sizeof(lampRoutines));
    routineCountIlumi = 0;
    currentRoutineIndexLamp = 0;

    const int startAddress = EEPROM_LAMP_ROUTINES;
    const int routineSize  = sizeof(LampRoutine);

    for (int i = 0; i < MAX_LAMP_ROUTINES; i++) {
      LampRoutine emptyRoutine = {0, 0, 0, 0, 0};
      EEPROM.put(startAddress + (i * routineSize), emptyRoutine);
    }
    EEPROM.commit(); // Aqui tudo bem, não é um loop rápido.
    Serial.println("Dados limpos (Delete).");
    ph.setFlagDeleteRoutineLamp(0);
  }

  // ==========================================
  // 2. RECEBENDO DADOS RAPIDAMENTE (RAM)
  // ==========================================
  if (lampRoutineMqtt && lampRoutineMqttTime) {
    unsigned long currentTimeLamp = millis();

    // Se faz mais de 10 segundos desde o último pacote, é um "Novo Lote"
    if (currentTimeLamp - lastPayloadLamp > 10000 || lastPayloadLamp == 0 ) {
      Serial.println("Limpando RAM para novo lote...");
      memset(lampRoutines, 0, sizeof(lampRoutines));
      routineCountIlumi = 0;
      currentRoutineIndexLamp = 0;

    }

    lastPayloadLamp = currentTimeLamp;
    lastStormTime = currentTimeLamp; // Marca o tempo da última gota da tempestade

    if (ph.instensityTimes[0] >= 0 && ph.instensityTimes[1] >= 0 && ph.instensityTimes[2] >= 0) {

      int colonIndex = ph.getTime().indexOf(':');
      LampRoutine newRoutine = {
        ph.instensityTimes[0],
        ph.instensityTimes[1],
        ph.instensityTimes[2],
        ph.getTime().substring(0, colonIndex).toInt(),
        ph.getTime().substring(colonIndex + 1).toInt()
      };

      lampRoutines[currentRoutineIndexLamp] = newRoutine;
      Serial.printf("Indice salvo na RAM: %d\n", currentRoutineIndexLamp);

      currentRoutineIndexLamp = (currentRoutineIndexLamp + 1) % 5;
      if (routineCountIlumi < 5) routineCountIlumi++;

      // Sinaliza que a RAM tem dados frescos, mas NÃO SALVA AINDA
      pendingEEPROMSave = true;
    }

    if (routineCountIlumi != 0) {
      routineIlu = true;
      prefs.putBool("routineLight", routineIlu);
      lightOn = false;
      prefs.putBool("lightOn", lightOn);
      statusIlum = "DESATIVADO";
      Serial.println("Ativando flags");
    }

    // Libera para o próximo MQTT
    ph.instensityTimes[0] = -1;
    ph.instensityTimes[1] = -1;
    ph.instensityTimes[2] = -1;
    ph.setFlagLampRoutine(0);
    ph.setFlagLampRoutineTime(0);
  }

  // ==========================================
  // 3. SALVAR NA EEPROM (APÓS A TEMPESTADE)
  // ==========================================
  // Se temos dados pendentes e já passou 1 segundo desde o último payload (fim da rajada)
  if (pendingEEPROMSave && (millis() - lastStormTime > 1000)) {
    Serial.println("Rajada MQTT finalizada! Salvando tudo na EEPROM agora...");

    // Agora sim, chamamos a sua função pesada 1 única vez!
    saveRoutinelight();
    prefs.putInt("indexLight", routineCountIlumi);
    pendingEEPROMSave = false; // Desativa a flag
  }

  if (routineIlu) {
    applyLastRoutine();
  }

  // 🔓 Tira o cadeado
  isProcessingLamp = false;
}

void showRoutinelight() {

  for (int i = 0; i < routineCountIlumi; i++) {
    LampRoutine routine = lampRoutines[i];
    Serial.print("Rotina ");
    Serial.print(i + 1);
    Serial.print(": Hora: ");
    Serial.print(routine.hour);
    Serial.print(", Minutos: ");
    Serial.print(routine.minutes);
    Serial.print(", Intensidade L1: ");
    Serial.print(routine.intensityL1);
    Serial.print(", Intensidade L2: ");
    Serial.print(routine.intensityL2);
    Serial.print(", Intensidade L3: ");
    Serial.println(routine.intensityL3);
  }
}

void lampImmediate() {
  lampImmediateMqtt = ph.getFlagLampInstant();
  if (lampImmediateMqtt) {

    priorityMqtt = false;
    displayPriority = true;
    if (ph.getIntensityL1 () >= 0) {
      Lamp1 = ph.getIntensityL1() / 10;
      updateLightIntensity(light1, Lamp1);
      intensityLight1 = Lamp1;
      ph.setIntensityL1(-1);
      previousRoundedValue = -1;
      Serial.println("Alterou l1");

    }

    if (ph.getIntensityL2() >= 0) {

      Lamp2 = ph.getIntensityL2() / 10;
      updateLightIntensity(light2, Lamp2);
      intensityLight2 = Lamp2;
      previousRoundedValue = -1;
      ph.setIntensityL2(-1);
      Serial.println("Alterou l2");
    }
    if (ph.getIntensityL3() >= 0) {

      Lamp3 = ph.getIntensityL3() / 10;
      updateLightIntensity(light3, Lamp3);
      intensityLight3 = Lamp3;
      previousRoundedValue = -1;
      ph.setIntensityL3(-1);
      Serial.println("Alterou l3");

    }
    ph.setFlagLampInstant(0);
    sendDataL1L2L3();
    //  sendDataL1();
    // sendDataL2();
    //  sendDataL3();
    checkAll();
  }
}

void feederImmediate() {
  feederNowMqtt = ph.getFlagFeederInstant();
  if (feederNowMqtt) {
    ph.setFlagFeederInstant(0);

    int feederNow = ph.getFeederSpinsInstant();

    // Verifica se o valor retornado é instantâneo
    if (feederNow > 0) {  // Supondo que 0 significa não instantâneo, ajuste conforme necessário
      sendDataFeederConfirm;
      activateFeederMqtt(feederNow);  // Aciona o feeder com o número de rotações especificado
      ph.setFeederSpinsInstant(0);  // Reseta o valor para evitar múltiplas ativações
    }
  }
}

void serverCreated() {
  bool isFlagServerCreated = ph.getFlagCreated();
  if (isFlagServerCreated) {
    isServerCreated = isFlagServerCreated;

    Serial.println("Servidor Criou o Dispositivo");
    // Salva no Preferences
    prefs.putBool("serverCreated", isServerCreated);
    ph.setFlagCreated(0);
    sendData();

  }
}

void tempAdjust () {
  tempAdjustMqtt = ph.getFlagTemperature();
  if (tempAdjustMqtt) {
    tempMqtt =  ph.getTemperature();
    rangeMqtt =  ph.getRange();
    if   (rangeMqtt > 0 && rangeMqtt <= 10  && tempMqtt > 0) {

      Serial.println("Ajustando");

      modificTempP = true;

      savetemp = tempMqtt;
      range = rangeMqtt;
      modificTempDTemp = savetemp;
      idealTemp = savetemp;

      // Salva tudo no Preferences
      prefs.putInt("idealTemp", savetemp);
      prefs.putFloat("range", rangeMqtt);
      prefs.putBool("tempModified", modificTempP);

      sendDataTempControllConfirm();
      sendDataTemp();

    }
    else {
      sendDataError2();
    }

    ph.setTemperature(0);
    ph.setRange(0);


  }
  ph.setFlagTemperature(0);

}


void resetHardPayload() {
  bool hard = ph.getFlagHardReset();
  if (hard) {
    Serial.println("Payload Recebido, iremos resetar");
    sendDataHardResetConfirm();
    ResetDefaut();
  }
  ph.setFlagHardReset(0);
}



void timeAjust() {
  timeAjustMqtt = ph.getFlagRtc();

  if (timeAjustMqtt) {
    rtcMqtt = ph.getTimeRtc(); // Substitua com sua função
    Serial.print("rtc");
    Serial.println(rtcMqtt);
    time_t now = myRTC.get();
    sendDataRTC();
    currentRtc = static_cast<unsigned long long>(now);
    // Calcula a diferença absoluta
    unsigned long long difference = std::llabs(static_cast<long long>(rtcMqtt) - static_cast<long long>(currentRtc));

    // Verifica se a diferença é maior ou igual a dois minutos (120 segundos)
    if (difference >= 1 && rtcMqtt > 0) {
      // myRTC.adjust(DateTime(F(__DATE__),F(__TIME__)));
      myRTC.set(rtcMqtt + 10);
      setTime(rtcMqtt);
      now = myRTC.get();  // Obtém o tempo atual do RTC


      vTaskDelay(pdMS_TO_TICKS(100));
      digitalClockDisplay();
      Serial.print("Buffer: ");
      Serial.println(bufferTime);  // Exibe o valor do buffer para verificação
      Serial.println("Hora alterada");
      //checkAll();
      ph.setTimeRtc(0);
    }
    ph.setFlagRtc(0);

  }

  //}
}


void signatureAdjust() {

  signatureAdjustMqtt = ph.getFlagSubscription();

  if (signatureAdjustMqtt) {

    Serial.println("Ajustando assinatura");

    SignatureMqtt = ph.getSubscription();

    // Salva no Preferences
    prefs.putULong64("subscription", SignatureMqtt);

    ph.setSubscription(0);
    sendDatasignatureConfirm();
    ph.setFlagSubscription(0);
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


String convertTimestampToTimeString(unsigned long long timestamp) {
  // Converte de unsigned long long para time_t
  time_t timeValue = (time_t)timestamp;

  // Converte o time_t em uma estrutura tm
  struct tm *timeinfo = gmtime(&timeValue);

  // Formata a hora e minuto como string "hh:mm"
  char timeString[6];  // Buffer para armazenar o resultado "hh:mm"
  snprintf(timeString, sizeof(timeString), "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);

  return String(timeString);  // Retorna a string formatada
}

char strValor[10];

void inicialization() {

  Serial.println("Verificando se ja foi inicializado");

  firstReset = prefs.getInt("firstReset", -1);

  Serial.print("firstReset: ");
  Serial.println(firstReset);

  if (firstReset == -1 || firstReset == 1) {

    Serial.println("Reset padrao aplicado");

    interval = 300000;
    memory.saveMemoryUL("timer", interval);
    // ===== CONFIG PADRÃO =====

    SignatureMqtt = 1804800852;
    prefs.putULong64("subscription", SignatureMqtt);

    routineIlu = false;
    prefs.putBool("routineLight", routineIlu);

    routineFeeder = false;
    prefs.putBool("routineFeeder", routineFeeder);

    modificTempP = false;
    prefs.putBool("tempModified", modificTempP);

    savetemp = 25;
    prefs.putInt("idealTemp", savetemp);

    range = 1.0;
    prefs.putFloat("range", range);

    contReset = 0;
    prefs.putInt("contReset", contReset);

    rotation = 2;
    prefs.putInt("rotation", rotation);

    isServerCreated = false;
    prefs.putBool("serverCreated", isServerCreated);

    lightOn = false;
    prefs.putBool("lightOn", lightOn);

    illuminationStartHour = 8;
    illuminationStartMinute = 0;
    illuminationEndHour = 20;
    illuminationEndMinute = 0;

    prefs.putInt("lightStartHour", illuminationStartHour);
    prefs.putInt("lightStartMinute", illuminationStartMinute);
    prefs.putInt("lightEndHour", illuminationEndHour);
    prefs.putInt("lightEndMinute", illuminationEndMinute);

    feedingHour = 12;
    feedingMinute = 0;
    feederOn = false;

    prefs.putInt("feedingHour", feedingHour);
    prefs.putInt("feedingMinute", feedingMinute);
    prefs.putBool("feederOn", feederOn);

  }

  // Marca como inicializado
  prefs.putInt("firstReset", 0);
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  memory.begin();

  prefs.begin("device", false);

  Wire.begin(I2C_SDA, I2C_SCL);
  setSyncProvider(myRTC.get);   // the function to get the time from the RTC
  if (timeStatus() != timeSet)
    Serial.println("Unable to sync with the RTC");
  else {
    Serial.println("RTC has set the system time");
  }
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)


  portENTER_CRITICAL_ISR(&mux); // Start of critical section
  portEXIT_CRITICAL_ISR(&mux); // End of critical section
  /*
    pinMode(buttonGo, INPUT);
    pinMode(buttonDown, INPUT);
    pinMode(buttonUp, INPUT);
    pinMode(resetButton, INPUT);
  */

  pinMode(buttonGo, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(resetButton, INPUT);

  // setTime(21,32,00,24,10,2024);
  myRTC.set(now());
  FlagBUp           = 0;// Esta flag inicia desativada
  attachInterrupt(digitalPinToInterrupt(buttonUp)      , interruptUp  , FALLING);// Interrupção externa do botão com a função up: o modo falling parece que não funciona direito, mas vida que segue
  FlagBDown           = 0;// Esta flag inicia desativada
  attachInterrupt(digitalPinToInterrupt(buttonDown)      , interruptDown  , FALLING);// Interrupção externa do botão com a função up: o modo falling parece que não funciona direito, mas vida que segue


  //  setTime(9, 58, 00, 26, 9, 2024);  // set the system time to 10h00m30s on 04Jul2024
  //  myRTC.set(now());                   // set the RTC from the system time
  WiFi.mode(WIFI_MODE_STA);
  vTaskDelay(pdMS_TO_TICKS(100));
  MACADRESS = WiFi.macAddress();
  MACADRESS.replace(":", "");   // substitui ":" por nada
  MACADRESS = MACADRESS.substring(5);
  Serial.println("MACADRESS");
  Serial.println(MACADRESS);
  vTaskDelay(pdMS_TO_TICKS(100));
  WiFi.mode(WIFI_OFF);


  xSemaphore = xSemaphoreCreateBinary();
  if (xSemaphore == NULL) {
    Serial.println("Erro ao criar o semáforo.");
    while (1);  // Para execução se houver erro
  }

  // Inicializa o semáforo como disponível
  xSemaphoreGive(xSemaphore);

  if (!ina219_1.begin()) Serial.println("INA219 #1 não encontrado");
  if (!ina219_2.begin()) Serial.println("INA219 #2 não encontrado");
  if (!ina219_3.begin()) Serial.println("INA219 #3 não encontrado");
  if (!ina219_5.begin()) Serial.println("INA219 #5 não encontrado");


  sensors.begin();
  // Configures all pins as output
  pinMode(feeder, OUTPUT);
  pinMode(switchFeeder , OUTPUT);
  pinMode(cooler, OUTPUT);
  pinMode(heater, OUTPUT);
  pinMode(light1 , OUTPUT);
  pinMode(light2, OUTPUT);
  pinMode(light3, OUTPUT);

  // Initializes all outputs to off
  digitalWrite(feeder, LOW);
  digitalWrite(switchFeeder , LOW);
  digitalWrite(cooler, LOW);
  digitalWrite(heater , LOW);
  digitalWrite(light1, LOW);
  digitalWrite(light2, LOW);
  digitalWrite(light3, LOW);

  //numProgram = 1;;
  // Configuration PWM
  ledcAttach(light1, freq, resolution);
  ledcAttach(light2, freq, resolution);
  ledcAttach(light3, freq, resolution);

  readTemp.attach_ms(15000, onSensorsIDTemp);
  errorProgram.attach_ms(5000, checkError);
  timeProgram.attach_ms(5000, digitalClockDisplay);
  periodicTemp.attach_ms(45000, controllTempWrapper);
  //periodicServer.attach_ms(25000, serverSendOK);
  Preprogram.attach_ms(25000, routineDisplay);

  Serial.setDebugOutput(true);
  vTaskDelay(pdMS_TO_TICKS(100));

  PayloadHandle::initializeServerHandle(&sh);

  std::string siteUrlStr = std::string(siteUrl.c_str());
  sh.setSiteUrl(siteUrlStr);


  setReconnect = memory.readMemoryBool("reconnect", false);
  rotation = prefs.getInt("rotation", 2);
  isServerCreated = prefs.getBool("serverCreated", false);

  tft.begin();
  tft.setSwapBytes(true);
  tft.setRotation(rotation);
  Serial.println("rotation");
  Serial.println(rotation);
  vTaskDelay(pdMS_TO_TICKS(100));

  tft.fillScreen(TFT_BLACK);
  vTaskDelay(pdMS_TO_TICKS(100));

  clearArea(0, 0, 240, 240, TFT_BLACK);
  showLogo();
  vTaskDelay(pdMS_TO_TICKS(5000));
  allocateImages();
  loadImages();
  modifyImages();


  std::string ssid_str = memory.readMemoryStr("ssid");
  std::string passwordWifi_str = memory.readMemoryStr("password");

  ssid = String(ssid_str.c_str());
  password = String(passwordWifi_str.c_str());

  Serial.print("ssid: ");
  Serial.println(ssid);
  Serial.print("password: ");
  Serial.println(password);
  Serial.print("addr password: ");
  Serial.println();

  Serial.println("setup");

  updateLightIntensity(light1, 0);
  updateLightIntensity(light2, 0);
  updateLightIntensity(light3, 0);


  // ===== TEMPERATURA =====
  modificTempDTemp = prefs.getInt("idealTemp", 25);
  Serial.println("modificTempDTemp:");
  Serial.println(modificTempDTemp);

  modificTempP = prefs.getBool("tempModified", false);
  Serial.println("modificTempP:");
  Serial.println(modificTempP);

  tempMqtt = prefs.getInt("idealTemp", 25);
  Serial.println("savetemp:");
  Serial.println(tempMqtt);

  rangeMqtt = prefs.getFloat("range", 1.0);
  Serial.println("rangeMqtt:");
  Serial.println(rangeMqtt);

  // ===== ROTINAS =====
  routineFeeder = prefs.getBool("routineFeeder", false);
  Serial.println("routineFeeder:");
  Serial.println(routineFeeder);

  routineIlu = prefs.getBool("routineLight", false);
  Serial.println("routineIlu:");
  Serial.println(routineIlu);

  // ===== ASSINATURA =====
  SignatureMqtt = prefs.getULong64("subscription", 0);
  Serial.println("SignatureMqtt:");
  Serial.println(SignatureMqtt);

  // ===== SERVIDOR =====
  serverInitialized = prefs.getBool("serverCreated", false);

  backupRoutinefeeder();
  showRoutinefeeder();
  backupRoutinelight();
  showRoutinelight();

  ph.instensityTimes[0] = -1;
  ph.instensityTimes[1] = -1;
  ph.instensityTimes[2] = -1;
  // ph.feederSpins[0]=-1;




  if (modificTempP == 1) {
    savetemp = modificTempDTemp;
    idealTemp = savetemp;
  }

  inicialization();


  xTaskCreatePinnedToCore(
    controllButton,
    "switchMenu",
    4096,
    NULL,
    1,
    &SwicthMenuHandle,
    1
  );

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


  //int addrSetTimer = memory.getAddrSetTimer();
  bool setTimer = memory.readMemoryBool("setTimer", false);

  if (setTimer) {
    Serial.println("Alterado o time");
    interval = memory.readMemoryUL("timer", 300000);
  }
  sh.setVersion(std::string(versionStr.c_str()));


  sendDataL1();
  vTaskDelay(500 / portTICK_PERIOD_MS);

  sendDataL2();
  vTaskDelay(500 / portTICK_PERIOD_MS);

  sendDataL3();
  onSensorsIDTemp();
  vTaskDelay(pdMS_TO_TICKS(200));
  displayTemp = temp;

  // ===== FEEDER =====
  feederOn      = prefs.getBool("feederOn", false);
  feedingHour   = prefs.getInt("feedingHour", 12);
  feedingMinute = prefs.getInt("feedingMinute", 0);

  // ===== ILUMINAÇÃO =====
  lightOn = prefs.getBool("lightOn", false);

  illuminationStartHour   = prefs.getInt("lightStartHour", 8);
  illuminationStartMinute = prefs.getInt("lightStartMinute", 0);

  illuminationEndHour     = prefs.getInt("lightEndHour", 20);
  illuminationEndMinute   = prefs.getInt("lightEndMinute", 0);
  Serial.println("feederOn: ");
  Serial.println(feederOn);
  lastfeederOn = feederOn;
  lastfeedingHour = feedingHour;
  lastfeedingMinute = feedingMinute;

  lastilluminationEndHour = illuminationEndHour;
  lastilluminationEndMinute = illuminationEndMinute;
  lastilluminationStartHour = illuminationStartHour;
  lastilluminationStartMinute = illuminationStartMinute;
  laststatusIlum = statusIlum;
  //isServerCreated = 1;
  feederMode = feederOn ? "  ATIVADO" : "DESATIVADO";
  statusIlum = lightOn ? "  ATIVADO" : "DESATIVADO";
  /*
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg_init = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg_init);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    vTaskDelay(pdMS_TO_TICKS(100));


    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL);
    esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);

    esp_smartconfig_set_type(SC_TYPE_ESPTOUCH);
    esp_smartconfig_start(&cfg);
  */
  WiFi.mode(WIFI_STA); // Garante que o Arduino preparou o WiFi no modo Station
  vTaskDelay(pdMS_TO_TICKS(100));
  // ==========================================
  // INICIA SMARTCONFIG APENAS SE FOR NECESSÁRIO
  // ==========================================
  if (!setReconnect) {
    Serial.println("\nAguardando credenciais via SmartConfig (Esptouch)...");


    wifi_init_config_t cfg_init = WIFI_INIT_CONFIG_DEFAULT();

    vTaskDelay(pdMS_TO_TICKS(100));


    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL);
    esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);

    esp_smartconfig_set_type(SC_TYPE_ESPTOUCH);
    esp_smartconfig_start(&cfg);;

  } else {
    Serial.println("\nModo Reconnect ativado! Ignorando SmartConfig.");

    // Aqui você chama a sua função que tenta reconectar com as credenciais salvas
    // Exemplo baseado na sua classe WifiHandle:
    // sh.reconnectWifi(&memory);
  };

  //savetemp=idealTemp;
  Serial.println("\nAguardando SmartConfig...");

  // ===== CONTADOR RESET =====
  contReset = prefs.getInt("contReset", 0);

  Serial.println("Quantas vezes foi resetado:");
  Serial.println(contReset);

  // Se está tentando reconectar
  if (setReconnect) {
    contReset++;
    prefs.putInt("contReset", contReset);
  }

  // Se passou do limite
  if (contReset > 3) {

    // ===== RESET RECONNECT (continua EEPROM se for MemoryManagment) =====
    setReconnect = false;
    memory.saveMemoryBool("reconnect", setReconnect);
    stopEspIDFStack();

    // ===== ZERA CONTADOR =====
    contReset = 0;
    prefs.putInt("contReset", contReset);

    Serial.print("ptr addr: ");
    Serial.println(addrReconnect);


    // ===== SERVER CREATED =====
    serverInitialized = false;
    prefs.putBool("serverCreated", serverInitialized);

    ESP.restart();
  }
}

unsigned long previousMillisReset = 0;  // Armazena o último tempo em que a variável foi resetada
const long intervalReset = 30000;       // Intervalo de 30 segundos (30.000 milissegundos)


int contInfo = 0;
/*
  void loop() {

  if (firstPression == false) {
    if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {

      clearArea(0, 0, 240, 240, TFT_BLACK);

      if (WiFi.status() == WL_CONNECTED and contInfo <= 1) {

        contInfo = contInfo + 1;
      }
      HTTPClient http;

      http.begin("http://clients3.google.com/generate_204");
      int httpCode = http.GET();
      //Serial.printf("Código HTTP: %d\n", httpCode);

      if ((WiFi.status() == WL_CONNECTED) && httpCode == 204) {
        drawWifi(true);
      }
      else {
        drawWifi(false);

      }

      time_t now = myRTC.get();
      currentRtc = static_cast<unsigned long long>(now);
      if (SignatureMqtt >= currentRtc) {
        drawService(true);
      }
      else {
        drawService(false);
      }

      if (controllError[0] == -1 || controllError[1] == -1 || controllError[2] == -1 || controllError[3] == -1 || controllError[4] == -1  || temp == -30) {
        drawError();
      }
      if (timeStatus() != timeSet) {
        writeHours("-:-");
      } else {
        writeHours(bufferTime);
      }

      drawFooter(coolerFooter, heaterFooter, feederFooter, lightFooter);
      //clearArea(0, 60, 240, 80, TFT_BLACK);



      xSemaphoreGive(xSemaphore);
    }

  }



  if (firstPression == false) {

    if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {
      if (WiFi.status() != WL_CONNECTED) {
        //Serial.println("Show Temp");

        if (temp == -30) {
          writeTemperature("-");
        }
        else {
          writeTemperature(String(temp, 1));
        }
      }
      if (WiFi.status() == WL_CONNECTED) {
        if (temp == -30) {
          writeTemperature(String("-"));
        }
        else {
          writeTemperature(String(displayTemp, 1));
        }
      }
      xSemaphoreGive(xSemaphore);
    }
    vTaskDelay(pdMS_TO_TICKS(5000));
  }

  if (firstPression == false) {


    if (intensityLight1 != intensityLight2 || intensityLight1 != intensityLight3 || intensityLight2 != intensityLight3) {
      if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {
        clearArea(0, 60, 240, 80, TFT_BLACK);

        showLamp(1, String(intensityLight1 * 10));
        xSemaphoreGive(xSemaphore);
      }
      vTaskDelay(pdMS_TO_TICKS(3000));
    }
  }

  if (firstPression == false) {


    if (intensityLight1 != intensityLight2 || intensityLight1 != intensityLight3 || intensityLight2 != intensityLight3) {
      if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {
        clearArea(0, 60, 240, 80, TFT_BLACK);

        showLamp(2, String(intensityLight2 * 10));
        xSemaphoreGive(xSemaphore);
      }
      vTaskDelay(pdMS_TO_TICKS(3000));
    }
  }

  if (firstPression == false) {


    if (intensityLight1 != intensityLight2 || intensityLight1 != intensityLight3 || intensityLight2 != intensityLight3) {
      if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {
        clearArea(0, 60, 240, 80, TFT_BLACK);

        showLamp(3, String(intensityLight3 * 10));
        xSemaphoreGive(xSemaphore);
      }
      vTaskDelay(pdMS_TO_TICKS(3000));
    }
  }

  if (firstPression == false) {


    if (intensityLight1 == intensityLight2 && intensityLight2 == intensityLight3) {
      if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {
        clearArea(0, 60, 240, 80, TFT_BLACK);

        showLamp(0, String(intensityLight1 * 10));
        xSemaphoreGive(xSemaphore);
      }
      vTaskDelay(pdMS_TO_TICKS(3000));
    }
  }

  if (firstPression == false) {

    //Serial.println("writetemp");
    if (xSemaphoreTake(xSemaphore, 10000 / portTICK_PERIOD_MS) == pdTRUE) {

      //Serial.println("writetemp");
      if (feederMode == "  ATIVADO") {
        clearArea(0, 70, 240, 90, TFT_BLACK);
        String horaMinuto = (feedingHour < 10 ? "0" : "") + String(feedingHour) + ":" +
                            (feedingMinute < 10 ? "0" : "") + String(feedingMinute);
        showFeeder(horaMinuto);
        vTaskDelay(pdMS_TO_TICKS(3000));

      }
      else if (routineFeeder) {
        clearArea(0, 70, 240, 90, TFT_BLACK);
        showFeeder(convertTimestampToTimeString(next));
        vTaskDelay(pdMS_TO_TICKS(3000));
      }
      else {
        clearArea(0, 70, 240, 90, TFT_BLACK);
        showFeeder(String("--:--"));
        vTaskDelay(pdMS_TO_TICKS(3000));
      }


      // }

      xSemaphoreGive(xSemaphore);

    }

  }



  feederRoutine();
  lampRoutine();
  Serial.println("Esotu no loop");

  if (smartConfigDone ) {
    Serial.println("Esotu no loop");
    smartConfigDone = false;
    stopEspIDFStack();
    vTaskDelay(pdMS_TO_TICKS(100));
    connectWithArduinoWiFi();
  }

  serverSetupComplete();

  UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(SwicthButtonHandle);
  highWaterMark = uxTaskGetStackHighWaterMark(SwicthMenuHandle);
  highWaterMark = uxTaskGetStackHighWaterMark(SwicthPayloadHandle);


  feederRoutine();
  lampRoutine();
  vTaskDelay(pdMS_TO_TICKS(300));

  checkError();
  checkAll();
  tmElements_t tm;
  myRTC.read(tm);
  rtcHour = tm.Hour;
  rtcMinutes = tm.Minute;
  Serial.println("Esotu no loop2");

  sprintf(bufferTime, "%02d:%02d", rtcHour, rtcMinutes);



  if (intensityLight1 > 0 || intensityLight2 > 0 || intensityLight3 > 0) {
    lightFooter = true;
  }
  else {
    lightFooter = false;
  }


  vTaskDelay(pdMS_TO_TICKS(100));
  serverSetupComplete();
  vTaskDelay(pdMS_TO_TICKS(100));

  teste = millis();

  if (WiFi.status() == WL_CONNECTED) {
    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); //Colore rosso
    pixels.show();

  }






  //////////////////////////////////////////////////////////Vefificar se ta funcionando
  if (feederActive && millis() - previousMillisFeeder >= intervalFeeder) {
    digitalWrite(feeder, LOW);
    feederActive = false;
    feederFooter = false;
    Serial.println("                                                  Desativando Alimentador");
    sendDataFeeder();
  }
  if (digitalRead(switchFeeder) == LOW) {
    digitalWrite(feeder, LOW);
    feederActive = false;       // Reseta o status caso o switch seja   ATIVADO
    feederFooter = false;
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (serverCreated) {
      sh.checkMQTTConnection();
      //sh.updateHandle();
      //payloadInfo();
    }



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
    pixels.setPixelColor(0, pixels.Color(255, 0, 0)); //Colore rosso
    pixels.show();


    buildTopic();

  }
  }


*/


// ============================================================
// VARIÁVEIS GLOBAIS DE CONTROLE (Coloque antes do void loop)
// ============================================================
unsigned long lastHeaderUpdate = 0;
unsigned long lastContentUpdate = 0;
unsigned long lastMqttCheck = 0;
unsigned long previousMillisFeederCheck = 0;

// Estados: 0=Temp, 1=Lamp1, 2=Lamp2, 3=Lamp3, 4=LampGlobal, 5=Feeder
int displayState = 0;
int lastMinute = -1;       // Para evitar redesenhar o relógio sem necessidade
bool startupCleared = false; // Controle da limpeza inicial

// Intervalos de tempo (em milissegundos)
const long INTERVAL_HEADER = 1000;  // Relógio e WiFi
const long INTERVAL_TEMP = 5000;    // Tempo que a temperatura fica na tela
const long INTERVAL_CONTENT = 3000; // Tempo das outras telas

// ============================================================
// LOOP PRINCIPAL
// ============================================================
void loop() {
  unsigned long currentMillis = millis();

  // ----------------------------------------------------------
  // 1. ROTINAS DE HARDWARE E REDE (Executa sempre, sem travar)
  // ----------------------------------------------------------

  feederRoutine();
  lampRoutine();
  // SmartConfig
  /*  if (smartConfigDone) {
     // Serial.println("SmartConfig Done");
      //smartConfigDone = false;
      //stopEspIDFStack();
      vTaskDelay(pdMS_TO_TICKS(100));
      //connectWithArduinoWiFi();
    }*/

  if (flagStartMQTT) {
    flagStartMQTT = false; // Abaixa a flag para rodar apenas uma vez

    Serial.println("Iniciando rotinas MQTT no loop...");
    buildTopic();
    sh.connectToMQTTServer();
  }
  serverSetupComplete();

  // Verificações de Conexão e MQTT
  if (WiFi.status() == WL_CONNECTED) {
    contReset = 0;
    prefs.putInt("contReset", contReset);
    // LED de Status (Verde)
    if (currentMillis - teste > 1000) {
      pixels.setPixelColor(0, pixels.Color(0, 255, 0));
      pixels.show();
      teste = currentMillis;
    }

    if (serverCreated) {
      sh.checkMQTTConnection();
    }

    // Envio periódico de dados (2 min)
    if (currentMillis - previousMillis >= interval) {
      Serial.println("Enviando dados periódicos");
      previousMillis = currentMillis;
      sendData();
      showRoutinelight();
      showRoutinefeeder();

    }

    if (isCreatedWifi) {
      sendDataIdcustomic();
      isCreatedWifi = false;
    }

    // Tratamento de Erros MQTT
    if (ph.getError() || ph.getErrorDeserialization()) {
      std::map<std::string, std::string> payloadMap;
      payloadMap["error"] = ph.getError() ? "1" : "2";
      sh.sendPayload(sh.buildPayload(payloadMap), sh.getTopicData());

      ph.setError(false);
      ph.setErrorDeserialization(false);



    }

    // Transmissão sob demanda
    if (sh.getTransmission()) {
      sh.setTransmission(false);
      sendData();
    }

    if (ConnectCont < 1) {
      ConnectCont++;
      payloadInfo();
      sendData();

      if (statusIlum == "  ATIVADO") {
        sendLampRoutine(illuminationStartHour, illuminationStartMinute, illuminationEndHour, illuminationEndMinute);
        vTaskDelay(500 / portTICK_PERIOD_MS);

      }
      else if (routineIlu) {
        for (int i = 0; i < currentRoutineIndexLamp; i++) {
          LampRoutine routine = lampRoutines[i];

          sendRoutineIlu(
            routine.hour,
            routine.minutes,
            routine.intensityL1,
            routine.intensityL2,
            routine.intensityL3
          );

          vTaskDelay(500 / portTICK_PERIOD_MS);

        }
      }
      if (feederMode == "  ATIVADO") {

        sendfeederRoutineDisplay(1, feedingHour, feedingMinute);
        vTaskDelay(500 / portTICK_PERIOD_MS);

      }

      else if (routineFeeder) {
        for (int i = 0; i < routineCountFeeder; i++) {
          FeederRoutine routine = feederRoutines[i];

          sendfeederRoutine(
            routine.spins,
            routine.hour,
            routine.minutes
          );

          vTaskDelay(500 / portTICK_PERIOD_MS);

        }
      }
    }

  } else {
    // Reconexão WiFi (LED Vermelho)
    // Nota: Idealmente mover para timer não bloqueante, mas mantido lógica original simplificada
    std::string mqttHostStr = std::string(mqttHost.c_str());
    std::string mqttPortStr = std::string(mqttPort.c_str());
    std::string mqttUserStr = std::string(mqttUser.c_str());
    std::string mqttPasswordStr = std::string(mqttPassword.c_str());

    sh.setMqttHost(mqttHostStr);
    sh.setMqttPort(mqttPortStr);
    sh.setMqttUser(mqttUserStr);
    sh.setMqttPassword(mqttPasswordStr);


    wh.reconnectWifi(&memory);
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.show();
    buildTopic();
  }

  // Lógica do Alimentador (Desligamento automático)
  if (feederActive && currentMillis - previousMillisFeeder >= intervalFeeder) {
    digitalWrite(feeder, LOW);
    feederActive = false;
    feederFooter = false;
    Serial.println("Desativando Alimentador");
    sendDataFeeder();
  }
  if (digitalRead(switchFeeder) == LOW) {
    digitalWrite(feeder, LOW);
    feederActive = false;
    feederFooter = false;
  }

  // ----------------------------------------------------------
  // 2. LÓGICA DE DISPLAY (Interface Gráfica)
  // ----------------------------------------------------------

  // A) LIMPEZA INICIAL DA TELA (Roda uma vez após sair da logo)
  if (!firstPression && !startupCleared) {
    if (xSemaphoreTake(xSemaphore, 100 / portTICK_PERIOD_MS) == pdTRUE) {
      clearArea(0, 0, 240, 240, TFT_BLACK); // Limpa a logo

      // TRUQUE: Força os timers para o passado para desenhar IMEDIATAMENTE
      lastHeaderUpdate = millis() - INTERVAL_HEADER - 100;
      lastContentUpdate = millis() - INTERVAL_TEMP - 100;
      lastMinute = -1;  // Força redesenho do relógio
      startupCleared = true;
      xSemaphoreGive(xSemaphore);
      Serial.println ("Limpando tela");
    }
    // Não damos return aqui para que ele já desenhe na sequência
  }

  // B) ATUALIZAÇÃO DA INTERFACE
  if (!firstPression && startupCleared) {

    // --- HEADER E FOOTER (A cada 1 segundo) ---
    if (currentMillis - lastHeaderUpdate >= INTERVAL_HEADER) {
      lastHeaderUpdate = currentMillis;

      if (xSemaphoreTake(xSemaphore, 100 / portTICK_PERIOD_MS) == pdTRUE) {

        // 1. Relógio (Com correção de sobreposição)
        tmElements_t tm;
        myRTC.read(tm);

        // Só redesenha se o minuto mudou ou status mudou
        if (tm.Minute != lastMinute ||
            timeStatus() != timeSet ||
            forceClockRedraw) {

          lastMinute = tm.Minute;
          forceClockRedraw = false;

          clearArea(140, 0, 120, 40, TFT_BLACK);
          sprintf(bufferTime, "%02d:%02d", tm.Hour, tm.Minute);
          if (timeStatus() != timeSet) {
            writeHours("-:-");
          } else {
            writeHours(bufferTime);
          }
        }

        // 2. WiFi Status Check
        HTTPClient http;
        http.begin("http://clients3.google.com/generate_204");
        int httpCode = (WiFi.status() == WL_CONNECTED) ? http.GET() : 0;

        if (WiFi.status() == WL_CONNECTED && contInfo <= 1) contInfo++;

        if (WiFi.status() == WL_CONNECTED && httpCode == 204) {
          drawWifi(true);
        } else {
          drawWifi(false);
        }
        http.end(); // Libera recursos

        // 3. MQTT Service Icon
        time_t now = myRTC.get();
        currentRtc = static_cast<unsigned long long>(now);
        drawService(SignatureMqtt >= currentRtc);

        // 4. Erros
        if (controllError[0] == -1 || temp == -30) {
          drawError();
        }

        // 5. Footer Icons
        lightFooter = (intensityLight1 > 0 || intensityLight2 > 0 || intensityLight3 > 0);
        drawFooter(coolerFooter, heaterFooter, feederFooter, lightFooter);

        xSemaphoreGive(xSemaphore);
      }
    }

    // --- CONTEÚDO CENTRAL (Carrossel) ---

    // Define tempo: Temperatura (5s), outros (3s)
    long currentInterval = (displayState == 0) ? INTERVAL_TEMP : INTERVAL_CONTENT;

    if (currentMillis - lastContentUpdate >= currentInterval) {
      lastContentUpdate = currentMillis;

      // Avança estado
      displayState++;
      if (displayState > 5) displayState = 0;

      if (xSemaphoreTake(xSemaphore, 100 / portTICK_PERIOD_MS) == pdTRUE) {

        bool didDraw = false;
        bool lampsUnified = (intensityLight1 == intensityLight2 && intensityLight2 == intensityLight3);

        // Área Central para limpar (Y=50 até Y=160 aprox)
        // Ajuste 'areaY' e 'areaH' para não apagar o Footer nem o Header
        int areaY = 50;
        int areaH = 110;

        switch (displayState) {
          case 0: // TEMPERATURA
            clearArea(0, areaY, 240, areaH, TFT_BLACK);
            if (WiFi.status() != WL_CONNECTED || temp == -30) {
              writeTemperature((temp == -30) ? "-" : String(temp, 1));
            } else {
              writeTemperature(String(displayTemp, 1));
            }
            didDraw = true;
            break;

          case 1: // LAMP 1
            if (!lampsUnified) {
              clearArea(0, areaY, 240, areaH, TFT_BLACK);
              showLamp(1, String(intensityLight1 * 10));
              didDraw = true;
            }
            break;

          case 2: // LAMP 2
            if (!lampsUnified) {
              clearArea(0, areaY, 240, areaH, TFT_BLACK);
              showLamp(2, String(intensityLight2 * 10));
              didDraw = true;
            }
            break;

          case 3: // LAMP 3
            if (!lampsUnified) {
              clearArea(0, areaY, 240, areaH, TFT_BLACK);
              showLamp(3, String(intensityLight3 * 10));
              didDraw = true;
            }
            break;

          case 4: // LAMP GLOBAL (Todas iguais)
            if (lampsUnified) {
              clearArea(0, areaY, 240, areaH, TFT_BLACK);
              showLamp(0, String(intensityLight1 * 10));
              didDraw = true;
            }
            break;

          case 5: // FEEDER INFO
            clearArea(0, areaY, 240, areaH, TFT_BLACK);
            if (feederMode == "  ATIVADO") {
              String horaMinuto = (feedingHour < 10 ? "0" : "") + String(feedingHour) + ":" + (feedingMinute < 10 ? "0" : "") + String(feedingMinute);
              showFeeder(horaMinuto);
            } else if (routineFeeder) {
              showFeeder(convertTimestampToTimeString(next));
            } else {
              showFeeder(String("--:--"));
            }
            didDraw = true;
            break;
        }

        xSemaphoreGive(xSemaphore);

        // SE NÃO DESENHOU (ex: pulou Lamp 2), força atualização imediata
        if (!didDraw) {
          lastContentUpdate = currentMillis - currentInterval - 100;
        }
      }
    }
  }

  // Debug e Checks Finais
  checkError();
  checkAll();

  // Atualiza variáveis globais de tempo se necessário para outras lógicas
  tmElements_t tm;
  myRTC.read(tm);
  rtcHour = tm.Hour;
  rtcMinutes = tm.Minute;
}
