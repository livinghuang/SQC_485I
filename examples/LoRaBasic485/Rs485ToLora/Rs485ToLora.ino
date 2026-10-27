/*
  RS485-to-LoRa example for the SQC_485I board.

  Receives data on RS485 and sends it as a LoRa (point-to-point, not LoRaWAN)
  packet. Pair it with the LoraToRs485 example on a second board.

  Board revision: set the #define below to match your board (see README).

  Arduino IDE: Tools > Board > Heltec ESP32 Series > Heltec Wireless Mini Shell
  Dependencies: "Heltec ESP32 Dev-Boards" library
    https://github.com/HelTecAutomation/Heltec_ESP32

  Use a frequency and output power that are legal in your country.
*/

// ---- Select your board revision (see README) ----
#define SQC485I_V2_3_3 // for a v1 board change this line to: #define SQC485I_V1
#include <SQC_485I.h>

#define RF_FREQUENCY 923000000 // Hz, adjust for your region
#define TX_OUTPUT_POWER 14     // dBm

#define LORA_BANDWIDTH 0        // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12]
#define LORA_CODINGRATE 1       // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH 8
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

#define RS485_BAUD 9600

SQC_485I board;
static RadioEvents_t RadioEvents;
bool lora_idle = true;

void OnTxDone()
{
  Radio.Sleep();
  SQC485I_CONSOLE.println("LoRa TX done.");
  lora_idle = true;
}

void OnTxTimeout()
{
  Radio.Sleep();
  SQC485I_CONSOLE.println("LoRa TX timeout.");
  lora_idle = true;
}

void setup()
{
  SQC485I_CONSOLE.begin(115200);
  board.begin();
  SQC485I_CONSOLE.println(SQC_485I::BOARD_NAME);

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

  board.rs485Begin(RS485_BAUD, SERIAL_8N1);
  board.rs485Send("Hello, I'm RS485 to LoRa!");
}

void loop()
{
  if (lora_idle)
  {
    String message = board.rs485ReceiveString();
    if (message.length() > 0)
    {
      SQC485I_CONSOLE.println("RS485 received: " + message);
      lora_idle = false;
      Radio.Send((uint8_t *)message.c_str(), message.length());
    }
  }
  Radio.IrqProcess();
  delay(1);
}
