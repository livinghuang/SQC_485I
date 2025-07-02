#ifndef SQC_485I_H
#define SQC_485I_H
#include "LoRaWan_APP.h"
#include <Arduino.h>

class SQC_485I
{
public:
  SQC_485I() {}

  void begin(uint32_t baud = 9600, bool USE_EXTERNAL_XTAL = 1)
  {
    Mcu.begin(HELTEC_BOARD, USE_EXTERNAL_XTAL);
  }
  void ledOn()
  {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // LOW = ON (Vext)
  }
  void ledOff()
  {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // HIGH = OFF
  }

  const uint8_t LED_PIN = 2;   // Vext 控制腳
  const uint8_t RS485_DE = 9;  // Data Enable
  const uint8_t RS485_RO = 19; // Receiver Output
  const uint8_t RS485_DI = 18; // Driver Input
};

#endif
