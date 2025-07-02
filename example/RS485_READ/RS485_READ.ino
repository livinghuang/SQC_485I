#include "LoRaWan_APP.h"
#include "Arduino.h"
#define USE_EXTERNAL_XTAL 1 // Use external crystal oscillator
#define pVext 2             // Vext connected to GPIO 2. It will light the LED when Vext is low

#define BAUD_RATE 9600
#define SERIAL_CONFIG SERIAL_8N1

#define pRS485_RO 19 // Receiver Output (RO) connected to GPIO 19
#define pRS485_DI 18 // Driver Input (DI) connected to GPIO 18
#define pRS485_DE 9  // Data Enable (DE) connected to GPIO 9, you need to replug in USB port

void setup()
{
  Serial.begin(115200); // Initialize Serial for debugging
  Mcu.begin(HELTEC_BOARD, USE_EXTERNAL_XTAL);

  pinMode(pVext, OUTPUT);
  digitalWrite(pVext, LOW);
  pinMode(pRS485_DE, OUTPUT);
  pinMode(pRS485_RO, INPUT);  // Receiver Output (RO) should be input
  pinMode(pRS485_DI, OUTPUT); // Driver Input (DI) should be output
  digitalWrite(pRS485_DE, LOW);
  Serial1.begin(BAUD_RATE, SERIAL_CONFIG, pRS485_RO, pRS485_DI); // Initialize RS485
}

void loop()
{
  String buffer;
  digitalWrite(pRS485_DE, LOW); // Enable RS485 reception
  delay(100);
  if (Serial1.available())
  {
    buffer = Serial1.readString();                         // Read data into buffer
    Serial.println("RX Mode: Received data -> " + buffer); // Print received data
  }
}
