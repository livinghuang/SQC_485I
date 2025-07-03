/*
  SQC_485I LoRa-to-RS485 Example

  This example demonstrates how to use the SQC_485I board to receive data via LoRa
  and forward it to an RS485 device. The sketch initializes the RS485 transceiver,
  sends a "Hello" message on startup, and continuously listens for incoming data
  from LoRa, forwarding it directly to the RS485 bus.

  Board:   SQC_485I (based on Heltec Wireless Mini - HT-CT62)
  Author:  Siliq
  Date:    2025

  Dependencies:
  - Heltec ESP32 LoRaWAN library (v3.0.2)
    https://github.com/HelTecAutomation/Heltec_ESP32
    Make sure this library is installed before compiling.

  ⚠️ Important Notes:
  - GPIO9 is shared between RS485 DE control and USB boot mode.
    After uploading code, RS485 communication may not work properly
    until you **unplug and replug the USB cable** to reset the pin state.
*/
#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <SQC_485I.h>

#define RF_FREQUENCY 923000000 // LoRa frequency (adjust for your region)
#define TX_OUTPUT_POWER 14     // dBm

#define LORA_BANDWIDTH 0        // [0: 125kHz, 1: 250kHz, 2: 500kHz]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12]
#define LORA_CODINGRATE 1       // [1: 4/5, ..., 4: 4/8]
#define LORA_PREAMBLE_LENGTH 8
#define LORA_SYMBOL_TIMEOUT 0
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

#define BUFFER_SIZE 64
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;
bool lora_idle = true;

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
void sendRS485(const String &message);

void setup()
{
  Serial.begin(115200);
  board.begin();

  // Initialize LoRa
  RadioEvents.RxDone = OnRxDone;
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetRxConfig(MODEM_LORA,
                    LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE,
                    0,
                    LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT,
                    LORA_FIX_LENGTH_PAYLOAD_ON,
                    0,
                    true,
                    0,
                    0,
                    LORA_IQ_INVERSION_ON,
                    true);

  // Initialize RS485
  beginRS485();

  // Send initial message
  sendRS485("Hello, I'm LoRa to RS485!");
}

void loop()
{
  if (lora_idle)
  {
    lora_idle = false;
    Serial.println("Waiting for LoRa packet...");
    Radio.Rx(0); // Continuous RX
  }
  Radio.IrqProcess(); // Handle LoRa IRQs
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
  memcpy(rxpacket, payload, size);
  rxpacket[size] = '\0';

  Radio.Sleep();
  Serial.printf("Received LoRa: \"%s\" | RSSI: %d | Size: %d\n", rxpacket, rssi, size);

  // Forward to RS485
  sendRS485(String(rxpacket)); // Convert to String for transmission

  lora_idle = true;
}

void beginRS485()
{
  // --- RS485 Pin Setup ---
  pinMode(pRS485_DE, OUTPUT);
  pinMode(pRS485_RO, INPUT);
  pinMode(pRS485_DI, OUTPUT);

  // --- Initialize RS485 UART (Serial1) ---
  Serial1.begin(BAUD_RATE, SERIAL_CONFIG, pRS485_RO, pRS485_DI);

  // --- Set RS485 to transmit mode initially ---
  digitalWrite(pRS485_DE, HIGH);
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
