
/*
  void readTemp(){
   sensors.requestTemperatures();

   delay(750);

   float tempC = sensors.getTempCByIndex(0);
   Serial.print("Temperature: ");
   Serial.print(tempC);
   Serial.println("°C");
   delay(1000);
  }
*/


/*
  void runThermostat() {
  // Só roda se tiver um AC selecionado e o modo automático estiver ativo
  if (activeAC == nullptr || !thermostatEnabled) return;

  // Verifica a cada 5 segundos para não travar o processador
  if (millis() - lastTempCheck > 5000) {
    lastTempCheck = millis();

    sensors.requestTemperatures();
    float currentTemp = sensors.getTempCByIndex(0);

    Serial.print("Atual: "); Serial.print(currentTemp);
    Serial.print(" | Alvo: "); Serial.println(targetTemp);

    // Histerese de 1 grau para evitar liga/desliga frenético
    if (currentTemp > (targetTemp + 0.5) && !acIsOn) {
      // Está quente, LIGA o ar
      Serial.println("Termostato: Ligando AC...");

      // Configura para FRIO e define a temperatura no driver
      activeAC->setCool(activeAC->obj);
      activeAC->setTemp(activeAC->obj, targetTemp); // Define o AC para a temp alvo
      activeAC->on(activeAC->obj); // Prepara comando ON

      activeAC->send(activeAC->obj); // Envia sinal IR
      acIsOn = true;

    } else if (currentTemp < (targetTemp - 0.5) && acIsOn) {
      // Está frio, DESLIGA o ar
      Serial.println("Termostato: Desligando AC...");

      activeAC->off(activeAC->obj); // Prepara comando OFF

      activeAC->send(activeAC->obj); // Envia sinal IR
      acIsOn = false;
    }
  }
  }
*/


void runThermostat() {
  if (activeAC == nullptr || !thermostatEnabled) return;
  static unsigned long lastLoop = 0;
  if (millis() - lastLoop < 5000) return;
  lastLoop = millis();

  // 1️⃣ Leitura do sensor
  sensors.requestTemperatures();
  T_raw = sensors.getTempCByIndex(0);

  // 2️⃣ Filtro exponencial
  T_filt = alpha * T_raw + (1 - alpha) * T_filt;

  if (last_read_time == 0) {
    last_read_time = millis();
    T_prev = T_filt;
    return;
  }

  // 3️⃣ Derivada
  float dt = (millis() - last_read_time) / 1000.0;
  if (dt > 0) {
    dTdt = (T_filt - T_prev) / dt * 60.0;
  }

  T_prev = T_filt;
  last_read_time = millis();

  // 4️⃣ Tempo de assentamento
  if (millis() - last_action_time < settling_time) return;

  float error = T_filt - targetTemp;

  // 🔻 Ambiente quente
  if (error > hysteresis) {

    if (dTdt < -0.2) return;

    if (millis() - last_action_time >= min_action_time) {

      AC_setpoint--;
      AC_setpoint = constrain(AC_setpoint, AC_MIN_TEMP, AC_MAX_TEMP);

      activeAC->setTemp(activeAC->obj, AC_setpoint);
      sendActive();

      last_action_time = millis();
    }
  }

  // 🔺 Ambiente frio
  else if (error < -hysteresis) {

    if (dTdt > 0.2) return;

    if (millis() - last_action_time >= min_action_time) {

      AC_setpoint++;
      AC_setpoint = constrain(AC_setpoint, AC_MIN_TEMP, AC_MAX_TEMP);

      activeAC->setTemp(activeAC->obj, AC_setpoint);
      sendActive();

      last_action_time = millis();
    }
  }

  // Debug
  Serial.printf(
    "T=%.2f | Set=%.2f | AC=%d | dT/dt=%.2f\n",
    T_filt, targetTemp, AC_setpoint, dTdt
  );
}


/*
  unsigned long lastTempChangeTime = 0;  // Marca o tempo da última mudança de temperatura
  float lastTemp = -127.0;               // Armazena a última temperatura lida
  const float maxTemp = 16;            // Temperatura máxima permitida
  const float minTemp = 31;             // Temperatura mínima permitida
  bool constTemp = false;                // Indica se a temperatura está constante
  bool controlActive = true;             // Indica se o controle de temperatura está ativo

  void controllTemp(float temp, float idealTemp, float range) {

  isTempValid = true;

  // Verifica se a temperatura está fora dos limites
  // Nao deve entrar aqui, pois devo adcionar uma verificação tambem na biblioteca, se ficar prolixo, retirar
  if (temp > maxTemp || temp < minTemp) {
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
*/
