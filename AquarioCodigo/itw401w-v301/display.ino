void allocateImages() {
  coolerCopy = new uint16_t[40 * 40];
  heaterCopy = new uint16_t[40 * 40];
  feederSmallCopy = new uint16_t[40 * 42];
  lampSmallCopy = new uint16_t[40 * 42];
  wifiCopy = new uint16_t[30 * 30];
  errorCopy = new uint16_t[30 * 30];
}

void loadImages() {
  memcpy_P(coolerCopy, coolerBit, sizeof(uint16_t) * 40 * 40);
  memcpy_P(heaterCopy, heaterBit, sizeof(uint16_t) * 40 * 40);
  memcpy_P(feederSmallCopy, feederSmallBit, sizeof(uint16_t) * 40 * 42);
  memcpy_P(lampSmallCopy, lampSmallBit, sizeof(uint16_t) * 40 * 42);
  memcpy_P(wifiCopy, wifiBit, sizeof(uint16_t) * 30 * 30);
  memcpy_P(errorCopy, errorBit, sizeof(uint16_t) * 30 * 30);

}

void modifyImages() {
  modifyImageColor(coolerCopy, 40, 40, TFT_DARKGREY);
  modifyImageColor(heaterCopy, 40, 40, TFT_DARKGREY);
  modifyImageColor(feederSmallCopy, 40, 42, TFT_DARKGREY);
  modifyImageColor(lampSmallCopy, 40, 42, TFT_DARKGREY);
  modifyImageColor(wifiCopy, 30, 30, TFT_GREEN);
  modifyImageColor(errorCopy, 30, 30, TFT_RED);

}

void clearArea(int x, int y, int w, int h, uint16_t color) {
  tft.fillRect(x, y, w, h, color);
}

void modifyImageColor(uint16_t* image, int width, int height, uint16_t newColor) {
  for (int i = 0; i < width * height; i++) {
    if (image[i] != 0x0000) {
      image[i] = newColor;
    }
  }
}

void drawFooter(bool isCoolerActive, bool isHeaterActive, bool isFeederActive, bool isLampActive) {
  modifyImageColor(coolerCopy, 40, 40, isCoolerActive ? TFT_CYAN : grayColor);
  modifyImageColor(heaterCopy, 40, 40, isHeaterActive ? TFT_CYAN : grayColor);
  modifyImageColor(feederSmallCopy, 40, 42, isFeederActive ? TFT_CYAN : grayColor);
  modifyImageColor(lampSmallCopy, 40, 42, isLampActive ? TFT_CYAN : grayColor);

  tft.pushImage(10, 170, 40, 40, coolerCopy);
  tft.pushImage(70, 170, 40, 40, heaterCopy);
  tft.pushImage(130, 170, 40, 42, feederSmallCopy);
  tft.pushImage(190, 170, 40, 42, lampSmallCopy);
}

void drawService(bool isActive)
{
  uint16_t* buffer = new uint16_t[30 * 30];
  if (isActive)
  {
    memcpy_P(buffer, linkConnectionBit, sizeof(uint16_t) * 30 * 30);
  }
  else
  {
    memcpy_P(buffer, linkSlashBit, sizeof(uint16_t) * 30 * 30);
  }
  tft.pushImage(95, 5, 30, 30, buffer);
  delete[] buffer;
}

void writeHours(String hours) {
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE);
  tft.setTextWrap(true);
  tft.setCursor(140, 10);
  tft.print(hours);
}

void showAbout(String version) {

  clearScreen();
  String mac = WiFi.macAddress();

  mac.replace(":", "");
  mac.remove(0, 5);

  tft.setTextSize(3);
  tft.setCursor(80, 20);
  tft.setTextColor(TFT_CYAN);
  tft.print("Sobre");
  int rectX = 20, rectY = 100, WIDTH_ITEM_MENU = 80, HEIGHT_ITEM_MENU = 35;

  tft.setCursor(rectX, rectY);
  tft.setTextColor(TFT_WHITE);
  tft.print("ID:");
  tft.print(MACADRESS);
  tft.setTextSize(2);
  tft.setCursor(rectX, rectY + 30);
  tft.print(version);

  rectX = 20, rectY = 210, WIDTH_ITEM_MENU = 190, HEIGHT_ITEM_MENU = 30;

  tft.setCursor(rectX + 10, rectY + 5);
  tft.setTextColor(TFT_WHITE);
  tft.print("Voltar");

  tft.drawRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);
  tft.fillRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(rectX + 10, rectY + 5);
  tft.setTextColor(TFT_BLACK);
  tft.print("Voltar");
}

void showFeeder(String hour) {
  tft.pushImage(0, 60, 77, 80, feederBit);
  tft.setTextSize(5);
  tft.setTextColor(TFT_WHITE);
  tft.setTextWrap(true);
  tft.setCursor(78, 80);
  tft.print(hour);
}


void showLamp(int lamp, String isLampActive) {
  tft.pushImage(0, 60, 77, 80, lampBit);

  // Configurações de texto principal
  tft.setTextSize(5);
  tft.setTextColor(TFT_WHITE);
  tft.setTextWrap(true);

  if (lamp == 0) {
    tft.setCursor(95, 80);
    tft.print(isLampActive);
  }

  clearArea(60, 130, 40, 40, TFT_BLACK);
  if (lamp == 1) {
    tft.setCursor(95, 80);
    tft.print(isLampActive);
    tft.setTextSize(3); // Tamanho menor para o número
    tft.setCursor(60, 130); // Ajusta a posição para o índice
    tft.print(String(lamp));
  }

  if (lamp == 2) {
    tft.setCursor(95, 80);
    tft.print(isLampActive);
    tft.setTextSize(3); // Tamanho menor para o número
    tft.setCursor(60, 130); // Ajusta a posição para o índice
    tft.print(String(lamp));
  }

  if (lamp == 3) {
    tft.setCursor(95, 80);
    tft.print(isLampActive);
    tft.setTextSize(3); // Tamanho menor para o número
    tft.setCursor(60, 130); // Ajusta a posição para o índice
    tft.print(String(lamp));
  }
  tft.setTextSize(5);
}


void writeTemperature(String temperatureValue) {
  tft.pushImage(0, 60, 50, 80, temperatureBit);
  tft.setTextSize(6);
  tft.setTextColor(TFT_WHITE);
  tft.setTextWrap(true);
  tft.setCursor(50, 80);
  tft.print(temperatureValue);

  int centerX = 210, centerY = 80, radius = 3, thickness = 3;

  drawCircleWithThickness(centerX, centerY, radius, thickness, TFT_WHITE);

  tft.setTextSize(4);
  tft.setTextColor(TFT_WHITE);
  tft.setTextWrap(true);
  tft.setCursor(215, 80);
  tft.print("C");
}

void drawCircleWithThickness(int x, int y, int radius, int thickness, uint16_t color) {
  for (int i = 0; i < thickness; i++) {
    tft.drawCircle(x, y, radius + i, color);
  }
}

void drawWifi(bool isActive) {
  modifyImageColor(wifiCopy, 30, 30, isActive ? TFT_GREEN : TFT_RED);
  tft.pushImage(55, 0, 30, 30, wifiCopy);
}

void drawMenu() {
  tft.setTextSize(3);
  tft.setCursor(80, 20);
  tft.setTextColor(TFT_CYAN);
  tft.print("Menu");
  int rectX = 20, rectY = 100, WIDTH_ITEM_MENU = 190, HEIGHT_ITEM_MENU = 35;

  tft.setTextColor(TFT_WHITE);

  if (menu > 0 && menu <= 3) {
    tft.setCursor(rectX + 5, rectY + 5);
    tft.print("Alimentar");
    tft.drawRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);
    Serial.println("Alimentar");

    rectY += HEIGHT_ITEM_MENU;
    tft.setCursor(rectX + 5, rectY + 5);
    tft.print("Iluminar");
    tft.drawRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);
    Serial.println("Iluminar");

    rectY += HEIGHT_ITEM_MENU;
    tft.setCursor(rectX + 5, rectY + 5);
    tft.print("Temp");
    tft.drawRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);
    Serial.println("Temperatura");
  }

  else if (menu >= 4 && menu <= 6) {
    tft.setCursor(rectX + 5, rectY + 5);
    tft.print("Rotacionar");
    tft.drawRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);
    Serial.println("Rotacionar");

    rectY += HEIGHT_ITEM_MENU;
    tft.setCursor(rectX + 5, rectY + 5);
    tft.print("Wifi");
    tft.drawRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);
    Serial.println("Wifi");

    rectY += HEIGHT_ITEM_MENU;
    tft.setCursor(rectX + 5, rectY + 5);
    tft.print("Sobre");
    tft.drawRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);
    Serial.println("Sobre");
  }

  else if (menu >= 7 && menu <= 9) {
    tft.setCursor(rectX + 5, rectY + 5);
    tft.print("Cadastro");
    tft.drawRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);
    Serial.println("Cadastro");

    rectY += HEIGHT_ITEM_MENU;
    tft.setCursor(rectX + 5, rectY + 5);
    tft.print("Restaurar");
    tft.drawRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);
    Serial.println("Restaurar");

    rectY += HEIGHT_ITEM_MENU;
    tft.setCursor(rectX + 5, rectY + 5);
    tft.print("Sair");
    tft.drawRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);
    Serial.println("Sair");
  }
}


void drawError() {
  tft.pushImage(15, 0, 30, 30, errorCopy);
}

void resetScreen() {

  tft.setTextSize(3);
  tft.setCursor(60, 20);
  tft.setTextColor(TFT_CYAN);
  tft.print("Wifi");

  if (WiFi.status() == WL_CONNECTED) {
    tft.setCursor(30, 105);
    tft.setTextSize(2.7);
    tft.setTextColor(TFT_WHITE);
    tft.print("Conectado em : ");
    tft.setCursor(30, 130);   // linha abaixo
    tft.print(ssid);


  }
  else {
    tft.setCursor(30, 105);
    tft.setTextSize(2.7);
    tft.setTextColor(TFT_WHITE);
    tft.print("Desconectado");
  }

  tft.setTextSize(3);

  int rectX = 20, rectY = 150, WIDTH_ITEM_MENU = 190, HEIGHT_ITEM_MENU = 35;
  tft.setCursor(rectX + 10, rectY + 5);
  tft.setTextColor(TFT_WHITE);
  tft.print("Resetar");

  tft.drawRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);

  rectY += HEIGHT_ITEM_MENU;
  tft.drawRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);
  tft.setCursor(rectX + 10, rectY + 5);
  tft.print("Voltar");

}

void defaultsScreen() {

  tft.setTextSize(3);
  tft.setCursor(40, 20);
  tft.setTextColor(TFT_CYAN);
  tft.print("Restaurar ");
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setTextFont(1);
  tft.setTextSize(2); // ou 3 se couber
  tft.setTextWrap(false);  // desliga quebra automática

  tft.setCursor(10, 90);
  tft.println("Os dados serao");
  tft.setCursor(10, 120);
  tft.println("perdidos.");
  tft.setCursor(10, 150);
  tft.println("Deseja restaurar?");

  tft.setTextSize(3);

  int rectX = 10, rectY = 200, WIDTH_ITEM_MENU = 50, HEIGHT_ITEM_MENU = 35;
  tft.setCursor(rectX - 5, rectY + 5);
  tft.setTextColor(TFT_WHITE);
  tft.print("Sim");

  rectX = 170, rectY = 200, WIDTH_ITEM_MENU = 50, HEIGHT_ITEM_MENU = 35;
  tft.setCursor(rectX - 20, rectY + 5);
  tft.setTextColor(TFT_WHITE);
  tft.print("Nao");

}

void selectItemMenuReset(int index, int item) {
  clearArea(0, 0, 240, 240, TFT_BLACK);
  resetScreen();

  int rectX = 20, rectY = 150;

  switch (item) {
    case 1:
      fillItemMenu("Resetar", rectX, rectY);
      putBar(225, 0);
      break;
    case 2:
      fillItemMenu("Voltar", rectX, rectY + HEIGHT_ITEM_MENU);
      putBar(225, 160);
      break;
  }
}


void selectItemMenuDefaults(int index, int item) {


  clearArea(0, 0, 240, 240, TFT_BLACK);
  defaultsScreen();
  int rectX = 20, rectY = 70;
  WIDTH_ITEM_MENU = 210;
  switch (item) {

    case 1:
      WIDTH_ITEM_MENU = 120;
      confirmLightFillItem("Sim", "", 5, 170 + HEIGHT_ITEM_MENU);
      break;
    case 2:
      WIDTH_ITEM_MENU = 120;
      confirmLightFillItem("Nao", "", 120, 170 + HEIGHT_ITEM_MENU);
      break;
    default:
      return;
  }

}

void rotationScreen() {

  tft.setTextSize(3);
  tft.setCursor(30, 20);
  tft.setTextColor(TFT_CYAN);
  tft.print("Rotacionar");
  Serial.println("Rotacionar");
  Serial.println(rotation);


  int rectX = 20, rectY = 100, WIDTH_ITEM_MENU = 190, HEIGHT_ITEM_MENU = 35;
  tft.setCursor(rectX + 10, rectY + 5);
  tft.setTextColor(TFT_WHITE);
  tft.print("Horizontal");

  tft.drawRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);

  if (rotation == 2) {
    tft.fillRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_SILVER);
    tft.setTextSize(3);
    tft.setCursor(rectX + 10, rectY + 5);
    tft.setTextColor(TFT_BLACK);
    tft.print("Horizontal");
  }

  rectY += HEIGHT_ITEM_MENU;
  tft.drawRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);
  tft.setCursor(rectX + 10, rectY + 5);
  tft.setTextColor(TFT_WHITE);
  tft.print("Torre");

  if (rotation == 3) {
    tft.fillRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_SILVER);
    tft.setTextSize(3);
    tft.setCursor(rectX + 10, rectY + 5);
    tft.setTextColor(TFT_BLACK);
    tft.print("Torre");
  }

  rectY += HEIGHT_ITEM_MENU;
  tft.drawRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);
  tft.setCursor(rectX + 10, rectY + 5);
  tft.setTextColor(TFT_WHITE);
  tft.print("Voltar");

}

void selectItemMenuRotation(int index, int item) {
  clearArea(0, 0, 240, 240, TFT_BLACK);
  rotationScreen();

  int rectX = 20, rectY = 100;

  switch (item) {
    case 1:
      fillItemMenu("Horizontal", rectX, rectY);
      putBar(225, 0);
      break;
    case 2:
      fillItemMenu("Torre", rectX, rectY + HEIGHT_ITEM_MENU);
      putBar(225, 80);
      break;
    case 3:
      fillItemMenu("Voltar", rectX, rectY + 2 * HEIGHT_ITEM_MENU);
      putBar(225, 160);
      break;
  }
}


void temperaturaScreen(String RangeScream) {

  tft.setTextSize(3);

  tft.setCursor(30, 20);
  tft.setTextColor(TFT_CYAN);
  tft.print("Temperatura");

  tft.pushImage(60, 80, 30, 30, arrowUpBit);
  tft.setTextSize(4);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(105, 135);
  tft.pushImage(60, 175, 30, 30, arrowDownBit);
  tft.setTextSize(2);

  tft.setCursor(0, 60);
  tft.print("Atual: ");
  tft.print((int)modificTempDTemp);
  tft.setCursor(118, 60);
  tft.print("+");  // ± em hexadecimal
  tft.setCursor(118, 70);
  tft.print("-");  // ± em hexadecimal
  tft.setCursor(135, 60);

  tft.print(range);  // ± em hexadecimal



  int centerX = 190, centerY = 60, radius = 2, thickness = 2;
  drawCircleWithThickness(centerX, centerY, radius, thickness, TFT_WHITE);
  tft.setCursor(195, 60);
  tft.print("C");
  tft.setCursor(60, 135);
  tft.setTextSize(3);

  tft.print(savetemp);


  tft.setCursor(130, 135);
  tft.print("+");  // ± em hexadecimal
  tft.setCursor(130, 145);
  tft.print("-");  // ± em hexadecimal
  tft.setCursor(150, 135);

  tft.print("1.00");  // ± em hexadecimal

  int rectX = 10, rectY = 200, WIDTH_ITEM_MENU = 50, HEIGHT_ITEM_MENU = 35;
  tft.setCursor(rectX - 5, rectY + 5);
  tft.setTextColor(TFT_WHITE);
  tft.print("Salvar");

  rectX = 150, rectY = 200, WIDTH_ITEM_MENU = 50, HEIGHT_ITEM_MENU = 35;
  tft.setCursor(rectX - 20, rectY + 5);
  tft.setTextColor(TFT_WHITE);
  tft.print("Voltar");



}

void cleanup() {

  delete[] coolerCopy;
  delete[] heaterCopy;
  delete[] feederSmallCopy;
  delete[] lampSmallCopy;
  delete[] wifiCopy;
  delete[] errorCopy;

}

void showLogo() {

  tft.pushImage(0, 0, 240, 240, mercyBit);

}


void showQrCode() {
  clearArea(0, 0, 240, 240, TFT_BLACK);
  tft.pushImage(20, 0, 200, 200, qrCodeBit);

  int rectX = 20, rectY = 210, WIDTH_ITEM_MENU = 190, HEIGHT_ITEM_MENU = 30;
  tft.setCursor(rectX + 10, rectY + 5);
  tft.setTextColor(TFT_WHITE);
  tft.print("Voltar");

  tft.drawRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);

  tft.fillRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(rectX + 10, rectY + 5);
  tft.setTextColor(TFT_BLACK);
  tft.print("Voltar");

}

void clearScreen() {
  tft.fillScreen(TFT_BLACK);
}

void selectItemMenu(int index, int item) {
  resetMenuItems(index);
  if (!index) {
    WIDTH_ITEM_MENU = 210;
    int rectX = 20, rectY = 100;
    if (menu > 0 && menu <= 3) {
      switch (item) {
        case 1:
          fillItemMenu("Alimentar", rectX, rectY );
          putBar(225, 0);
          break;
        case 2:
          fillItemMenu("Iluminar", rectX, rectY +  HEIGHT_ITEM_MENU);
          putBar(225, 23);
          break;
        case 3:
          fillItemMenu("Temp", rectX, rectY + 2 * HEIGHT_ITEM_MENU);
          putBar(225, 49);
          break;
      }
    }
    else if (menu >= 4 && menu <= 6 ) {
      WIDTH_ITEM_MENU = 210;
      rectX = 20, rectY = 100;
      switch (item) {
        case 4:
          fillItemMenu("Rotacionar", rectX, rectY);
          putBar(225, 49);
          break;
        case 5:
          fillItemMenu("Wifi", rectX, rectY +  HEIGHT_ITEM_MENU);
          putBar(225, 75);
          break;
        case 6:
          fillItemMenu("Sobre", rectX, rectY +  2 * HEIGHT_ITEM_MENU );
          putBar(225, 101);
          break;
      }
    }
    else if (menu >= 7 && menu <= 9 ) {
      WIDTH_ITEM_MENU = 210;
      rectX = 20, rectY = 100;
      switch (item) {
        case 7:
          fillItemMenu("Cadastro", rectX, rectY);
          putBar(225, 127);
          break;
        case 8:
          fillItemMenu("Restaurar", rectX, rectY + HEIGHT_ITEM_MENU);
          putBar(225, 140);
          break;
        case 9:
          fillItemMenu("Sair", rectX, rectY + 2 * HEIGHT_ITEM_MENU);
          putBar(225, 160);
          break;
      }

    }

  }
  else {
    return;
  }
}

void resetMenuItems(int menu) {

  clearArea(0, 0, 240, 240, TFT_BLACK);
  drawMenu();
}


void selectOption(int menu, int item) {
  if (menu == 1) {
    int rectX = 20, rectY = 70;
    WIDTH_ITEM_MENU = 210;
    switch (item) {
      case 1:
        feederFillItem("", feederMode, rectX, rectY);
        break;
      case 2:
        WIDTH_ITEM_MENU = 44;
        feederFillItem((feedingHour < 10 ? "0" : "") + String(feedingHour), "", 90, 100 + HEIGHT_ITEM_MENU);
        break;
      case 3:
        WIDTH_ITEM_MENU = 44;
        feederFillItem((feedingMinute < 10 ? "0" : "") + String(feedingMinute), "", 150, 100 + HEIGHT_ITEM_MENU);
        break;
      case 4:
        WIDTH_ITEM_MENU = 120;
        feederFillItem("Salvar", "", 10, 170 + HEIGHT_ITEM_MENU);
        break;
      case 5:
        WIDTH_ITEM_MENU = 100;
        feederFillItem("Voltar", "", 150, 170 + HEIGHT_ITEM_MENU);
        break;
      default:
        return;
    }
  }
  if (menu == 0) {
    int rectX = 20, rectY = 70;
    WIDTH_ITEM_MENU = 210;
    switch (item) {
      case 1:
        lightFillItem("", statusIlum, rectX, rectY);
        break;
      case 2:
        WIDTH_ITEM_MENU = 45;
        lightFillItem((illuminationStartHour < 10 ? "0" : "") + String(illuminationStartHour), "", 128, 75 + HEIGHT_ITEM_MENU);
        break;
      case 3:
        WIDTH_ITEM_MENU = 45;
        lightFillItem((illuminationStartMinute < 10 ? "0" : "") + String(illuminationStartMinute), "", 183, 75 + HEIGHT_ITEM_MENU);
        break;
      case 4:
        WIDTH_ITEM_MENU = 45;
        lightFillItem((illuminationEndHour < 10 ? "0" : "") + String(illuminationEndHour), "", 87, 115 + HEIGHT_ITEM_MENU);
        break;
      case 5:
        WIDTH_ITEM_MENU = 45;
        lightFillItem((illuminationEndMinute < 10 ? "0" : "") + String(illuminationEndMinute), "", 150, 115 + HEIGHT_ITEM_MENU);
        break;
      case 6:
        WIDTH_ITEM_MENU = 120;
        lightFillItem("Salvar", "", 10, 170 + HEIGHT_ITEM_MENU);
        break;
      case 7:
        WIDTH_ITEM_MENU = 100;
        lightFillItem("Voltar", "", 150, 170 + HEIGHT_ITEM_MENU);
        break;
      default:
        return;
    }
  }
  if (menu == 2) {
    int rectX = 20, rectY = 70;
    WIDTH_ITEM_MENU = 210;
    switch (item) {

      case 1:
        WIDTH_ITEM_MENU = 45;
        lightFillItem(String(savetemp), "", 60, 135);
        break;
      case 2:
        WIDTH_ITEM_MENU = 120;
        lightFillItem("Salvar", "", 10, 170 + HEIGHT_ITEM_MENU);
        break;
      case 3:
        WIDTH_ITEM_MENU = 100;
        lightFillItem("Voltar", "", 150, 170 + HEIGHT_ITEM_MENU);
        break;
      default:
        return;
    }
  }
}


//  tft.setCursor(60, 135);

void fillItemMenu(String word, int rectX, int rectY)
{
  tft.fillRect(rectX, rectY, WIDTH_ITEM_MENU - 20, HEIGHT_ITEM_MENU, TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(rectX + 2, rectY + 5);
  tft.setTextColor(TFT_BLACK);
  tft.print(word);
}


void putBar(int rectX, int rectY) {
  tft.fillRect(rectX, rectY, 8, 80, TFT_WHITE);
}

void selectItemMenuProg(int menu, int item) {

  resetMenuItemsProg(menu);

  if (menu == 1) {
    int rectX = 20, rectY = 70;
    WIDTH_ITEM_MENU = 210;
    switch (item) {
      case 1:
        confirmFeederFillItem("", feederMode, rectX, rectY);
        break;
      case 2:
        WIDTH_ITEM_MENU = 44;
        confirmFeederFillItem((feedingHour < 10 ? "0" : "") + String(feedingHour), "", 90, 100 + HEIGHT_ITEM_MENU);
        break;
      case 3:
        WIDTH_ITEM_MENU = 44;
        confirmFeederFillItem((feedingMinute < 10 ? "0" : "") + String(feedingMinute), "", 150, 100 + HEIGHT_ITEM_MENU);
        break;
      case 4:
        WIDTH_ITEM_MENU = 120;
        confirmFeederFillItem("Salvar", "", 5, 170 + HEIGHT_ITEM_MENU);
        break;
      case 5:
        WIDTH_ITEM_MENU = 120;
        confirmFeederFillItem("Voltar", "", 120, 170 + HEIGHT_ITEM_MENU);
        break;
      default:
        return;
    }

  } else if (menu == 0) {


    int rectX = 20, rectY = 70;
    WIDTH_ITEM_MENU = 210;
    switch (item) {
      case 1:
        confirmLightFillItem("", statusIlum, rectX, rectY);
        break;
      case 2:
        WIDTH_ITEM_MENU = 45;
        confirmLightFillItem((illuminationStartHour < 10 ? "0" : "") + String(illuminationStartHour), "", 128, 75 + HEIGHT_ITEM_MENU);
        break;
      case 3:
        WIDTH_ITEM_MENU = 45;
        confirmLightFillItem((illuminationStartMinute < 10 ? "0" : "") + String(illuminationStartMinute), "", 183, 75 + HEIGHT_ITEM_MENU);
        break;
      case 4:
        WIDTH_ITEM_MENU = 45;
        confirmLightFillItem((illuminationEndHour < 10 ? "0" : "") + String(illuminationEndHour), "", 86, 115 + HEIGHT_ITEM_MENU);
        break;
      case 5:
        WIDTH_ITEM_MENU = 45;
        confirmLightFillItem((illuminationEndMinute < 10 ? "0" : "") + String(illuminationEndMinute), "", 150, 115 + HEIGHT_ITEM_MENU);
        break;
      case 6:
        WIDTH_ITEM_MENU = 120;
        confirmLightFillItem("Salvar", "", 5, 170 + HEIGHT_ITEM_MENU);
        break;
      case 7:
        WIDTH_ITEM_MENU = 120;
        confirmLightFillItem("Voltar", "", 120, 170 + HEIGHT_ITEM_MENU);
        break;
      default:
        return;
    }
  }

  else if (menu == 2) {
    int rectX = 20, rectY = 70;
    WIDTH_ITEM_MENU = 210;

    switch (item) {
      case 1:
        WIDTH_ITEM_MENU = 45;
        confirmLightFillItem(String(savetemp), "",  60, 135);
        break;
      case 2:
        WIDTH_ITEM_MENU = 120;
        confirmLightFillItem("Salvar", "", 5, 170 + HEIGHT_ITEM_MENU);
        break;
      case 3:
        WIDTH_ITEM_MENU = 120;
        confirmLightFillItem("Voltar", "", 120, 170 + HEIGHT_ITEM_MENU);
        break;

      default:
        return;
    }
  }

  else {
    return;
  }

}

void resetMenuItemsProg(int menu) {

  clearArea(0, 0, 240, 240, TFT_BLACK);

  if (menu == 1) {
    feederProgramScreen();
  }
  if (menu == 0) {
    lightProgramScreen();
  }
  if (menu == 2) {
    temperaturaScreen("1.00");
  }
  else {
  }
}

void confirmLightFillItem(String wordMenu, String wordStatus, int rectX, int rectY)
{

  tft.fillRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(rectX + 10, rectY + 5);
  tft.setTextColor(TFT_BLACK);
  tft.print(wordMenu);
  tft.print(wordStatus);

}


void confirmFeederFillItem(String wordMenu, String wordStatus, int rectX, int rectY)
{

  tft.fillRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(rectX + 10, rectY + 5);

  tft.setTextColor(TFT_BLACK);
  tft.print(wordMenu);
  tft.print(wordStatus);

}

void feederFillItem(String wordMenu, String wordStatus, int rectX, int rectY)
{

  tft.fillRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, (TFT_GREY));
  tft.setTextSize(3);
  tft.setCursor(rectX + 10, rectY + 5);

  tft.setTextColor(TFT_WHITE);
  tft.print(wordMenu);
  tft.print(wordStatus);

}


void lightFillItem(String wordMenu, String wordStatus, int rectX, int rectY)
{

  tft.fillRect(rectX, rectY, WIDTH_ITEM_MENU, HEIGHT_ITEM_MENU, (TFT_GREY));
  tft.setTextSize(3);
  tft.setCursor(rectX + 10, rectY + 5);
  tft.setTextColor(TFT_WHITE);
  tft.print(wordMenu);
  tft.print(wordStatus);

}

void feederProgramScreen() {

  tft.setTextSize(3);

  tft.setCursor(20, 20);
  tft.setTextColor(TFT_CYAN);
  tft.print("Alimentacao");

  int rectX = 20, rectY = 70, WIDTH_ITEM_MENU = 210, HEIGHT_ITEM_MENU = 35;
  tft.setCursor(rectX + 10, rectY + 5);
  tft.setTextColor(TFT_WHITE);
  tft.print(feederMode);

  rectY += HEIGHT_ITEM_MENU * 2;


  tft.setCursor(rectX + 70, rectY + 5);
  tft.print((feedingHour < 10 ? "0" : "") + String(feedingHour));
  tft.setCursor(112, 145);
  tft.print(" : ");
  tft.setCursor(150, 145);
  tft.print((feedingMinute < 10 ? "0" : "") + String(feedingMinute));
  rectY += HEIGHT_ITEM_MENU;

  rectX = 10, rectY = 200, WIDTH_ITEM_MENU = 50, HEIGHT_ITEM_MENU = 35;
  tft.setCursor(rectX - 5, rectY + 5);
  tft.setTextColor(TFT_WHITE);
  tft.print("Salvar");

  rectX = 150, rectY = 200, WIDTH_ITEM_MENU = 50, HEIGHT_ITEM_MENU = 35;
  tft.setCursor(rectX - 20, rectY + 5);
  tft.setTextColor(TFT_WHITE);
  tft.print("Voltar");
}



void lightProgramScreen() {

  tft.setTextSize(3);

  tft.setCursor(20, 20);
  tft.setTextColor(TFT_CYAN);
  tft.print("Iluminacao");

  int rectX = 20, rectY = 70, WIDTH_ITEM_MENU = 210, HEIGHT_ITEM_MENU = 35;
  tft.setCursor(rectX + 10, rectY + 5);
  tft.setTextColor(TFT_WHITE);
  tft.print(statusIlum);

  rectY += 50  ;
  tft.setCursor(8 , rectY );
  tft.print("Inicio: ");
  tft.setCursor(133 , rectY );
  tft.print((illuminationStartHour < 10 ? "0" : "") + String(illuminationStartHour));
  tft.print(":");
  tft.print((illuminationStartMinute < 10 ? "0" : "") + String(illuminationStartMinute));

  rectY += 35;
  tft.setCursor(8, rectY);
  tft.print("Fim: ");
  tft.print((illuminationEndHour < 10 ? "0" : "") + String(illuminationEndHour));
  tft.print(":");
  tft.print((illuminationEndMinute < 10 ? "0" : "") + String(illuminationEndMinute));


  rectY += HEIGHT_ITEM_MENU;

  rectX = 10, rectY = 200, WIDTH_ITEM_MENU = 50, HEIGHT_ITEM_MENU = 35;
  tft.setCursor(rectX - 5, rectY + 5);
  tft.setTextColor(TFT_WHITE);
  tft.print("Salvar");

  rectX = 150, rectY = 200, WIDTH_ITEM_MENU = 50, HEIGHT_ITEM_MENU = 35;
  tft.setCursor(rectX - 20, rectY + 5);
  tft.setTextColor(TFT_WHITE);
  tft.print("Voltar");
}
