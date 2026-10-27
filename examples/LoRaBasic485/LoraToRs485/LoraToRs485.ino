/*
  LoRa-to-RS485 example for the SQC_485I board.

  Receives LoRa (point-to-point, not LoRaWAN) packets and forwards the payload
  to the RS485 bus. Pair it with the Rs485ToLora example on a second board.

  Board revision: set the #define below to match your board (see README).

  Arduino IDE: Tools > Board > Heltec ESP32 Series > Heltec Wireless Mini Shell
  Dependencies: "Heltec ESP32 Dev-Boards" library
    https://github.com/HelTecAutomation/Heltec_ESP32
*/

// ---- Select your board revision (see README) ----
#define SQC485I_V2_3_3 // for a v1 board change this line to: #define SQC485I_V1
#include <SQC_485I.h>

#define RF_FREQUENCY 923000000 // Hz, must match the sender

#define LORA_BANDWIDTH 0        // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz]
#define LORA_SPREADING_FACTOR 7 // [SF7..SF12]
#define LORA_CODINGRATE 1       // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH 8
#define LORA_SYMBOL_TIMEOUT 0
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

#define RS485_BAUD 9600
#define BUFFER_SIZE 256

SQC_485I board;
static RadioEvents_t RadioEvents;
bool lora_idle = true;

uint8_t rxpacket[BUFFER_SIZE];
volatile uint16_t rxsize = 0;

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
  if (size > BUFFER_SIZE)
    size = BUFFER_SIZE;
  memcpy(rxpacket, payload, size);
  rxsize = size;
  Radio.Sleep();
  SQC485I_CONSOLE.printf("LoRa received %u bytes, RSSI %d, SNR %d\n", size, rssi, snr);
  lora_idle = true;
}

void setup()
{
  SQC485I_CONSOLE.begin(115200);
  board.begin();
  SQC485I_CONSOLE.println(SQC_485I::BOARD_NAME);

  RadioEvents.RxDone = OnRxDone;
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

  board.rs485Begin(RS485_BAUD, SERIAL_8N1);
  board.rs485Send("Hello, I'm LoRa to RS485!");
}

void loop()
{
  if (rxsize > 0)
  {
    board.rs485Send(rxpacket, rxsize); // forward to RS485
    rxsize = 0;
  }
  if (lora_idle)
  {
    lora_idle = false;
    Radio.Rx(0); // continuous RX
  }
  Radio.IrqProcess();
}
