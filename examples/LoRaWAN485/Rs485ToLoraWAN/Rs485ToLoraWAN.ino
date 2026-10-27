/*
  RS485-to-LoRaWAN example for the SQC_485I board.

  Data received on RS485 is sent as the next LoRaWAN uplink. When nothing new
  has arrived, the uplink carries a 4-byte heartbeat (00 01 02 03).

  Board revision: set the #define below to match your board (see README).

  Arduino IDE:
    Tools > Board > Heltec ESP32 Series > Heltec Wireless Mini Shell
    Tools > LoRaWan Region           : your region (e.g. REGION_AS923)
    Tools > LoRaWan DevEUI           : CUSTOM or Generate By ChipID
  Dependencies: "Heltec ESP32 Dev-Boards" library
    https://github.com/HelTecAutomation/Heltec_ESP32

  Fill in your own OTAA (devEui / appEui / appKey) or ABP keys from your
  LoRaWAN network server. The all-zero values below are placeholders.
*/

// ---- Select your board revision (see README) ----
#define SQC485I_V2_3_3 // for a v1 board change this line to: #define SQC485I_V1
#include <SQC_485I.h>

/* OTAA parameters */
uint8_t devEui[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t appEui[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t appKey[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* ABP parameters */
uint8_t nwkSKey[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t appSKey[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint32_t devAddr = (uint32_t)0x00000000;

/* LoRaWAN channel mask, default channels 0-7 */
uint16_t userChannelsMask[6] = {0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

/* Region: selected in Tools > LoRaWan Region */
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

/* Class A and Class C are supported */
DeviceClass_t loraWanClass = CLASS_A;

/* Uplink interval in ms */
uint32_t appTxDutyCycle = 15000;

/* OTAA or ABP */
bool overTheAirActivation = true;

/* ADR enable */
bool loraWanAdr = true;

/* Confirmed or unconfirmed uplinks */
bool isTxConfirmed = true;

/* Application port */
uint8_t appPort = 2;

/* Number of transmission trials for confirmed uplinks */
uint8_t confirmedNbTrials = 4;

#define RS485_BAUD 9600

SQC_485I board;
String rs485_received = "";

static void prepareTxFrame(uint8_t port)
{
  if (rs485_received.length() > 0)
  {
    appDataSize = rs485_received.length();
    if (appDataSize > LORAWAN_APP_DATA_MAX_SIZE)
      appDataSize = LORAWAN_APP_DATA_MAX_SIZE;
    memcpy(appData, rs485_received.c_str(), appDataSize);
    rs485_received = "";
  }
  else
  {
    appDataSize = 4;
    appData[0] = 0x00;
    appData[1] = 0x01;
    appData[2] = 0x02;
    appData[3] = 0x03;
  }
}

void setup()
{
  SQC485I_CONSOLE.begin(115200);
  board.begin();
  SQC485I_CONSOLE.println(SQC_485I::BOARD_NAME);

  board.rs485Begin(RS485_BAUD, SERIAL_8N1);
  board.rs485Send("Hello, I'm RS485 to LoRaWAN!");

  deviceState = DEVICE_STATE_INIT;
}

void loop()
{
  // Collect RS485 data between LoRaWAN cycles (latest message wins)
  String message = board.rs485ReceiveString();
  if (message.length() > 0)
  {
    SQC485I_CONSOLE.println("RS485 received: " + message);
    rs485_received = message;
  }

  switch (deviceState)
  {
  case DEVICE_STATE_INIT:
  {
#if (LORAWAN_DEVEUI_AUTO)
    LoRaWAN.generateDeveuiByChipID();
#endif
    LoRaWAN.init(loraWanClass, loraWanRegion);
    LoRaWAN.setDefaultDR(3); // join DR, and DR when ADR is off
    break;
  }
  case DEVICE_STATE_JOIN:
  {
    LoRaWAN.join();
    break;
  }
  case DEVICE_STATE_SEND:
  {
    prepareTxFrame(appPort);
    LoRaWAN.send();
    deviceState = DEVICE_STATE_CYCLE;
    break;
  }
  case DEVICE_STATE_CYCLE:
  {
    txDutyCycleTime = appTxDutyCycle + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);
    LoRaWAN.cycle(txDutyCycleTime);
    deviceState = DEVICE_STATE_SLEEP;
    break;
  }
  case DEVICE_STATE_SLEEP:
  {
    LoRaWAN.sleep(loraWanClass);
    break;
  }
  default:
  {
    deviceState = DEVICE_STATE_INIT;
    break;
  }
  }
}
