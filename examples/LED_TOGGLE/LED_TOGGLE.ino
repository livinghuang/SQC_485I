/*
  LED toggle example for the SQC_485I board.

  Board revision: set the #define below to match your board (see README).
    v2.3.3: the white LED shares GPIO2 with the RS485 /RE pin.
            LED on = RS485 receiver on, LED off = receiver off.
    v1:     the LED is driven by the Vext power switch (GPIO2, active LOW).

  Arduino IDE: Tools > Board > Heltec ESP32 Series > Heltec Wireless Mini Shell
  Dependencies: "Heltec ESP32 Dev-Boards" library
    https://github.com/HelTecAutomation/Heltec_ESP32
*/

// ---- Select your board revision (see README) ----
#define SQC485I_V2_3_3 // for a v1 board change this line to: #define SQC485I_V1
#include <SQC_485I.h>

SQC_485I board;

void setup()
{
  SQC485I_CONSOLE.begin(115200);
  board.begin();
  SQC485I_CONSOLE.println(SQC_485I::BOARD_NAME);
}

void loop()
{
  board.ledOn();
  delay(1000);
  board.ledOff();
  delay(1000);
}
