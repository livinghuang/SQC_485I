#include "Arduino.h"
#include <SQC_485I.h>

SQC_485I board;
#define BAUD_RATE 9600
#define SERIAL_CONFIG SERIAL_8N1

#define pRS485_RO 19 // Receiver Output (RO) connected to GPIO 19
#define pRS485_DI 18 // Driver Input (DI) connected to GPIO 18
#define pRS485_DE 9  // Data Enable (DE) connected to GPIO 9, you need to replug in USB port

#define time_for_RS485_switch 100 // Time to switch RS485 mode in milliseconds

void setup()
{
  Serial.begin(115200); // Initialize Serial for debugging
  board.begin();        // Initialize board

  pinMode(pRS485_DE, OUTPUT);
  pinMode(pRS485_RO, INPUT);                                     // Receiver Output (RO) should be input
  pinMode(pRS485_DI, OUTPUT);                                    // Driver Input (DI) should be output
  Serial1.begin(BAUD_RATE, SERIAL_CONFIG, pRS485_RO, pRS485_DI); // Initialize RS485
  digitalWrite(pRS485_DE, HIGH);
  delay(time_for_RS485_switch); // Wait for RS485 to stabilize
  Serial1.print("Hello");
  Serial1.flush();
  delay(1);                     // wait for final byte to be sent
  digitalWrite(pRS485_DE, LOW); // Switch back to receive mode
}

void loop()
{
  String buffer;
  digitalWrite(pRS485_DE, LOW); // Enable RS485 reception
  delay(time_for_RS485_switch); // Allow time for the RS485 to switch to receive mode
  if (Serial1.available())
  {
    buffer = Serial1.readString();                         // Read data into buffer
    Serial.println("RX Mode: Received data -> " + buffer); // Print received data
    // ECHO
    digitalWrite(pRS485_DE, HIGH);
    delay(time_for_RS485_switch);
    Serial1.println(buffer);
    Serial1.flush();
    delay(1);                     // wait for final byte to be sent
    digitalWrite(pRS485_DE, LOW); // Switch back to receive mode
  }
}
