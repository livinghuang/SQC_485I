/*
  RS485 echo example for the SQC_485I board.

  Sends "Hello" on the RS485 bus at start-up, then echoes back every message
  it receives. The library handles the DE / RE direction pins for each board
  revision, so the same sketch runs on v1 and v2.3.3.

  Board revision: set the #define below to match your board (see README).

  Arduino IDE: Tools > Board > Heltec ESP32 Series > Heltec Wireless Mini Shell
  Dependencies: "Heltec ESP32 Dev-Boards" library
    https://github.com/HelTecAutomation/Heltec_ESP32

  Notes:
  - v1: GPIO9 is shared between RS485 DE and USB boot mode. If RS485 does not
    work right after flashing, unplug and replug the USB cable.
  - v2.3.3: GPIO9 is also the user key. Holding the key forces the RS485
    driver on, and holding it during reset enters download mode.
*/

// ---- Select your board revision (see README) ----
#define SQC485I_V2_3_3 // for a v1 board change this line to: #define SQC485I_V1
#include <SQC_485I.h>

SQC_485I board;

#define RS485_BAUD 9600

void setup()
{
  SQC485I_CONSOLE.begin(115200);
  board.begin();
  board.rs485Begin(RS485_BAUD, SERIAL_8N1);

  SQC485I_CONSOLE.println(SQC_485I::BOARD_NAME);
  board.rs485Send("Hello");
}

void loop()
{
  String message = board.rs485ReceiveString();
  if (message.length() > 0)
  {
    SQC485I_CONSOLE.println("RS485 received: " + message);
    board.rs485Send(message); // echo back
  }
  delay(10);
}
