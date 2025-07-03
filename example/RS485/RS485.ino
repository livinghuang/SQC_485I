/*
  RS485 Echo Example for SQC_485I Board

  This example demonstrates RS485 communication using the SQC_485I custom board.
  It initializes the RS485 transceiver, sends a "Hello" message, and then continuously
  listens for incoming data. When data is received, it echoes the message back.

  Board:     SQC_485I (based on Heltec Wireless Mini - HT-CT62)
  Author:    Siliq
  Date:      2025

  IMPORTANT:
  1. This sketch depends on the Heltec ESP32 LoRaWAN library (version 3.0.2).
     Please install it before using this example:
     https://github.com/HelTecAutomation/Heltec_ESP32

  2. GPIO9 is shared between RS485 DE control and USB boot mode.
     After flashing the program, RS485 control may not work correctly until you **replug the USB cable**.
*/

#include "Arduino.h"
#include <SQC_485I.h>

SQC_485I board;

// --- RS485 Parameters ---
#define BAUD_RATE 9600
#define SERIAL_CONFIG SERIAL_8N1

// RS485 control pins (match SQC_485I schematic)
#define pRS485_RO 19 // Receiver Output (RO)
#define pRS485_DI 18 // Driver Input (DI)
#define pRS485_DE 9  // Data Enable (DE), shared with USB boot

#define time_for_RS485_switch 100 // Time in ms to switch RS485 direction

void setup()
{
  Serial.begin(115200); // Serial debug output
  board.begin();        // Initialize SQC_485I board (includes Vext and LoRaWAN)

  // --- RS485 Pin Setup ---
  pinMode(pRS485_DE, OUTPUT);
  pinMode(pRS485_RO, INPUT);
  pinMode(pRS485_DI, OUTPUT);

  // --- Initialize RS485 UART (Serial1) ---
  Serial1.begin(BAUD_RATE, SERIAL_CONFIG, pRS485_RO, pRS485_DI);

  // --- Send initial message ---
  digitalWrite(pRS485_DE, HIGH); // Set to transmit mode
  delay(time_for_RS485_switch);
  Serial1.print("Hello");
  Serial1.flush();
  delay(1);                     // Ensure transmission completes
  digitalWrite(pRS485_DE, LOW); // Switch back to receive mode
}

void loop()
{
  String buffer;

  digitalWrite(pRS485_DE, LOW); // Ensure RS485 in receive mode
  delay(time_for_RS485_switch); // Allow direction change to stabilize

  if (Serial1.available())
  {
    buffer = Serial1.readString(); // Read incoming message
    Serial.println("RX Mode: Received data -> " + buffer);

    // --- Echo back received message ---
    digitalWrite(pRS485_DE, HIGH); // Switch to transmit
    delay(time_for_RS485_switch);
    Serial1.println(buffer);
    Serial1.flush();
    delay(1);
    digitalWrite(pRS485_DE, LOW); // Back to receive mode
  }
}
