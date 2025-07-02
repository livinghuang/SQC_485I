/*
This example demonstrates how to toggle the LED on the SQC_485I board.

It initializes the board and toggles the LED state every second.
The LED is controlled through the Vext pin (GPIO2), using the `SQC_485I` library to simplify access.

⚠️ Note: The `SQC_485I` library depends on the **Heltec LoRaWAN Library** provided by Heltec.
Please make sure you have installed the Heltec LoRaWAN Library before running this example.

You can find the library here:
👉 https://github.com/HelTecAutomation/Heltec_ESP32

*/
#include <SQC_485I.h>

SQC_485I board;

void setup()
{
  Serial.begin(115200); // Initialize Serial for debugging
  board.begin();        // Initialize board
}

void loop()
{
  board.ledOn(); // Turn on Vext/LED
  delay(1000);
  board.ledOff();
  delay(1000); // Turn off Vext/LED
}