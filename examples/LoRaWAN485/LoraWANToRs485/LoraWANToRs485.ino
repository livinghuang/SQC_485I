/*
  LoRaWAN-to-RS485 example for the SQC_485I board.

  Sends a 4-byte heartbeat uplink (00 01 02 03) every cycle and writes every
  downlink payload received from the network server to the RS485 bus as-is.
  Use Class C (below) if downlinks must arrive without waiting for an uplink.

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

static void prepareTxFrame(uint8_t port)
{
  appDataSize = 4;
  appData[0] = 0x00;
  appData[1] = 0x01;
  appData[2] = 0x02;
  appData[3] = 0x03;
}

// Called by the LoRaWAN stack for every downlink with payload
void downLinkDataHandle(McpsIndication_t *mcpsIndication)
{
  SQC485I_CONSOLE.printf("Downlink %s, %d bytes, port %d: ",
                         mcpsIndication->RxSlot ? "RXWIN2" : "RXWIN1",
                         mcpsIndication->BufferSize, mcpsIndication->Port);
  for (uint8_t i = 0; i < mcpsIndication->BufferSize; i++)
    SQC485I_CONSOLE.printf("%02X", mcpsIndication->Buffer[i]);
  SQC485I_CONSOLE.println();

  board.rs485Send(mcpsIndication->Buffer, mcpsIndication->BufferSize);
}

void setup()
{
  SQC485I_CONSOLE.begin(115200);
  board.begin();
  SQC485I_CONSOLE.println(SQC_485I::BOARD_NAME);

  board.rs485Begin(RS485_BAUD, SERIAL_8N1);
  board.rs485Send("Hello, I'm LoRaWAN to RS485!");

  deviceState = DEVICE_STATE_INIT;
}

void loop()
{
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
