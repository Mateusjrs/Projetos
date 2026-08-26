#include "Multiplexador.h"
int pcfAddress; // Intern variable .cpp
int len_pcf = 8;
int BuffPcf[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void MultiplexadorBufferHandle::zerarBuffer(int *pcfBuffer[]){
    for(int bites=0; bites<8; bites++)
    {
      *pcfBuffer[bites]=0;
    }
}
void MultiplexadorHandle::choiceMultiplexador(int *pcfAdressDec){
    pcfAddress = *pcfAdressDec;
}
int MultiplexadorHandle::converterBynToDec(int buff[]){
    float dec = 0;
    for(int bits=0; bits<8; bits++){
        if(buff[bits]==1){
            dec +=  pow(2,bits);
            dec = round(dec);
        }
    }
    return dec;
}

/*  activate the pcf's output (0 to 7) is necessary use a value in Wire.write(value):
PCF8574A
Port | Value
-------------
P0   |   1
P1   |   2
P2   |   4
P3   |   8
P4   |   16
P5   |   32
P6   |   64
P7   |   128

*/
void MultiplexadorHandle::transmissionToMux(){
    int decimal = converterBynToDec(BuffPcf);
    Wire.beginTransmission(pcfAddress);   
    Wire.write(decimal);
    Wire.endTransmission();
}
void MultiplexadorHandle::writePcfBuffer(int onOff, int port){
    for(int bits=0; bits<len_pcf; bits++){ //activate only P0
        if(bits==port){
            BuffPcf[bits]=onOff;
        }
    } 
}

int MultiplexadorHandle::readPcfBuffer(int address, int port){    // HIGH = 1, LOW  = 0 , ERROR = -1
    Wire.requestFrom(address, 1);
    if (Wire.available()) {
        int data = Wire.read();
        return (data & (1 << port)) ? 1 : 0;
    } else {
        return -1; 
    }
}