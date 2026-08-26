
#ifndef Multiplexer_h
#define Multiplexer_h
#include <Arduino.h> // Include the Arduino library if necessary

#include <Wire.h>
#include <cmath>
#include <cstring>


class MultiplexadorBufferHandle{
    private:
        void zerarBuffer(int *pcfBuffer[]);

};
class MultiplexadorHandle{
    public:
        void choiceMultiplexador(int *pcfAdressDec);
        void writePcfBuffer(int onOff, int port);
        void transmissionToMux();
        int readPcfBuffer(int address, int port); 

    private:
        int converterBynToDec(int buff[]);


};
#endif 