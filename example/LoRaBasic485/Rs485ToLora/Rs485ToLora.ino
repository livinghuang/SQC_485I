/*
  SQC_485I RS485-to-LoRa Example

  This example receives data via RS485 and sends it over LoRa using the SQC_485I board.
  It is based on the Heltec LoRaSender.ino example and uses the Heltec LoRaWAN library.

  Board:   SQC_485I (based on Heltec Wireless Mini - HT-CT62)
  Author:  Siliq
  Date:    2025

  Dependencies:
  - Heltec ESP32 LoRaWAN Library (v3.0.2)
    https://github.com/HelTecAutomation/Heltec_ESP32

  ⚠️ Important:
  - GPIO9 is shared between RS485 DE and USB boot.
    Please replug the USB after flashing to restore RS485 control.
*/

#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <SQC_485I.h>

#define RF_FREQUENCY 923000000 // LoRa frequency (adjust for your region)
#define TX_OUTPUT_POWER 14     // dBm

#define LORA_BANDWIDTH 0        // 125kHz
#define LORA_SPREADING_FACTOR 7 // SF7
#define LORA_CODINGRATE 1       // 4/5
#define LORA_PREAMBLE_LENGTH 8
#define LORA_SYMBOL_TIMEOUT 0
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

SQC_485I board;

// --- RS485 Parameters ---
#define BAUD_RATE 9600
#define SERIAL_CONFIG SERIAL_8N1

// RS485 control pins (match SQC_485I schematic)
#define pRS485_RO 19 // Receiver Output (RO)
#define pRS485_DI 18 // Driver Input (DI)
#define pRS485_DE 9  // Data Enable (DE), shared with USB boot

#define time_for_RS485_switch 100 // Time in ms to switch RS485 direction
void beginRS485();
String receiveRS485();
void sendRS485(const String &message);
static RadioEvents_t RadioEvents;

void setup()
{
  Serial.begin(115200);
  board.begin();

  // Initialize LoRa
  RadioEvents.TxDone = OnTxDone;
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA,
                    TX_OUTPUT_POWER,
                    0,
                    LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH,
                    LORA_FIX_LENGTH_PAYLOAD_ON,
                    true,
                    0,
                    0,
                    LORA_IQ_INVERSION_ON,
                    3000); // timeout
  beginRS485();            // Initialize RS485
}

void loop()
{
  String message = receiveRS485();

  if (message != "")
  {
    Serial.print("RS485 received: ");
    Serial.println(message);

    Radio.Send((uint8_t *)message.c_str(), message.length());
    Serial.println("LoRa sent.");
    delay(100);
  }
  delay(10);
}

void OnTxDone()
{
  Radio.Sleep();
  Serial.println("LoRa TX done.");
}

void beginRS485()
{
  // --- RS485 Pin Setup ---
  pinMode(pRS485_DE, OUTPUT);
  pinMode(pRS485_RO, INPUT);
  pinMode(pRS485_DI, OUTPUT);

  // --- Initialize RS485 UART (Serial1) ---
  Serial1.begin(BAUD_RATE, SERIAL_CONFIG, pRS485_RO, pRS485_DI);

  // --- Set RS485 to send mode initially ---
  sendRS485("Hello, I'm Rs485 to LoRa!"); // Initial message

  // --- Set RS485 to receive mode initially ---
  digitalWrite(pRS485_DE, LOW);
  delay(time_for_RS485_switch);
}

void sendRS485(const String &message)
{
  // --- Send initial message ---
  digitalWrite(pRS485_DE, HIGH); // Set to transmit mode
  delay(time_for_RS485_switch);
  Serial1.print(message);
  Serial1.flush();
  delay(1); // Ensure transmission completes
}

String receiveRS485()
{
  String buffer = "";
  // --- Send initial message ---
  digitalWrite(pRS485_DE, LOW); // Set to transmit mode
  delay(time_for_RS485_switch);
  if (Serial1.available())
  {
    buffer = Serial1.readString(); // Read incoming message
    Serial.println("RS485 Received data : " + buffer);
  }
  return buffer;
}
