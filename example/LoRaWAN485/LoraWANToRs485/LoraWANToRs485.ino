/*
  SQC_485I LoRaWAN-to-RS485 Example

  This example receives data via RS485 and sends it over LoRaWAN using the SQC_485I board.
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

// if true, next uplink will add MOTE_MAC_DEVICE_TIME_REQ
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

/* OTAA para*/
uint8_t devEui[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
uint8_t appEui[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t appKey[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* ABP para*/
uint8_t nwkSKey[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t appSKey[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint32_t devAddr = (uint32_t)0x00000000;

/*LoraWan channelsmask, default channels 0-7*/
uint16_t userChannelsMask[6] = {0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

/*LoraWan region, select in arduino IDE tools
 * AS band on 923MHz
LORAMAC_REGION_AS923,
 * Australian band on 915MHz
LORAMAC_REGION_AU915,
 * Chinese band on 470MHz
LORAMAC_REGION_CN470,
 * Chinese band on 779MHz
LORAMAC_REGION_CN779,
 * European band on 433MHz
LORAMAC_REGION_EU433,
 * European band on 868MHz
LORAMAC_REGION_EU868,
 * South korean band on 920MHz
LORAMAC_REGION_KR920,
 * India band on 865MHz
LORAMAC_REGION_IN865,
 * North american band on 915MHz
LORAMAC_REGION_US915,
 * North american band on 915MHz with a maximum of 16 channels
LORAMAC_REGION_US915_HYBRID,
 * Australian band on 915MHz Subband 2
LORAMAC_REGION_AU915_SB2,
 * AS band on 922.0-923.4MHz
LORAMAC_REGION_AS923_AS1,
 * AS band on 923.2-924.6MHz
LORAMAC_REGION_AS923_AS2
*/

LoRaMacRegion_t loraWanRegion = LORAMAC_REGION_AS923_AS2; // Change to your region, e.g., LORAMAC_REGION_AS923_AS1, LORAMAC_REGION_AS923_AS2 etc.

/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t loraWanClass = CLASS_A;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 15000;

/*OTAA or ABP*/
bool overTheAirActivation = true;

/*ADR enable*/
bool loraWanAdr = true;

/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = true;

/* Application port */
uint8_t appPort = 2;
/*!
 * Number of trials to transmit the frame, if the LoRaMAC layer did not
 * receive an acknowledgment. The MAC performs a datarate adaptation,
 * according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
 * to the following table:
 *
 * Transmission nb | Data Rate
 * ----------------|-----------
 * 1 (first)       | DR
 * 2               | DR
 * 3               | max(DR-1,0)
 * 4               | max(DR-1,0)
 * 5               | max(DR-2,0)
 * 6               | max(DR-2,0)
 * 7               | max(DR-3,0)
 * 8               | max(DR-3,0)
 *
 * Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
 * the datarate, in case the LoRaMAC layer did not receive an acknowledgment
 */
uint8_t confirmedNbTrials = 4;

/* Prepares the payload of the frame */
static void prepareTxFrame(uint8_t port)
{
  /*appData size is LORAWAN_APP_DATA_MAX_SIZE which is defined in "commissioning.h".
   *appDataSize max value is LORAWAN_APP_DATA_MAX_SIZE.
   *if enabled AT, don't modify LORAWAN_APP_DATA_MAX_SIZE, it may cause system hanging or failure.
   *if disabled AT, LORAWAN_APP_DATA_MAX_SIZE can be modified, the max value is reference to lorawan region and SF.
   *for example, if use REGION_CN470,
   *the max value for different DR can be found in MaxPayloadOfDatarateCN470 refer to DataratesCN470 and BandwidthsCN470 in "RegionCN470.h".
   */
  appDataSize = 4;
  appData[0] = 0x00;
  appData[1] = 0x01;
  appData[2] = 0x02;
  appData[3] = 0x03;
}

// downlink data handle function example
void downLinkDataHandle(McpsIndication_t *mcpsIndication)
{
  Serial.printf("+REV DATA:%s,RXSIZE %d,PORT %d\r\n", mcpsIndication->RxSlot ? "RXWIN2" : "RXWIN1", mcpsIndication->BufferSize, mcpsIndication->Port);
  Serial.print("+REV DATA:");
  String received_data = "";
  for (uint8_t i = 0; i < mcpsIndication->BufferSize; i++)
  {
    received_data += String(mcpsIndication->Buffer[i], HEX);
    Serial.printf("%02X", mcpsIndication->Buffer[i]);
  }
  Serial.println();
  sendRS485("\r\ngot data:" + received_data);
  uint32_t color = mcpsIndication->Buffer[0] << 16 | mcpsIndication->Buffer[1] << 8 | mcpsIndication->Buffer[2];
#if (LoraWan_RGB == 1)
  turnOnRGB(color, 5000);
  turnOffRGB();
#endif
}

void setup()
{
  Serial.begin(115200);
  board.begin();
  beginRS485();
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
    // both set join DR and DR when ADR off
    LoRaWAN.setDefaultDR(3);
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
    // Schedule next packet transmission
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

void beginRS485()
{
  // --- RS485 Pin Setup ---
  pinMode(pRS485_DE, OUTPUT);
  pinMode(pRS485_RO, INPUT);
  pinMode(pRS485_DI, OUTPUT);

  // --- Initialize RS485 UART (Serial1) ---
  Serial1.begin(BAUD_RATE, SERIAL_CONFIG, pRS485_RO, pRS485_DI);

  // --- Set RS485 to send mode initially ---
  sendRS485("Hello, I'm LoRaWAN to Rs485!"); // Initial message

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
