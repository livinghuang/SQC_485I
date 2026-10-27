/*
  SQC_485I board support library

  Supported hardware revisions (select ONE before including this header):

    #define SQC485I_V2_3_3     // SQC485I v2.3.3 (current production)
    #define SQC485I_V1         // SQC485I v1 (default when nothing is defined)
    #include <SQC_485I.h>

  If no revision is defined, the library falls back to v1 so that existing
  v1 sketches keep compiling unchanged, and prints a compile-time note.

  All board-dependent code lives in this header (inline), because Arduino
  compiles library .cpp files without seeing the sketch's #define.
*/
#ifndef SQC_485I_H
#define SQC_485I_H

#include "LoRaWan_APP.h"
#include <Arduino.h>

#if defined(SQC485I_V2_3_3) && defined(SQC485I_V1)
#error "SQC_485I: define only one of SQC485I_V2_3_3 / SQC485I_V1"
#endif

#if !defined(SQC485I_V2_3_3) && !defined(SQC485I_V1)
#pragma message("SQC_485I: no board revision defined, using SQC485I_V1. Add '#define SQC485I_V2_3_3' before '#include <SQC_485I.h>' for v2.3.3 boards.")
#define SQC485I_V1
#endif

#if defined(SQC485I_V2_3_3)
#include "HWCDC.h"

// v2.3.3 talks to the PC through the ESP32-C3 native USB (GPIO18/19).
// The Heltec "Wireless Mini Shell" board definition builds with
// "USB CDC On Boot" disabled, so `Serial` would go to UART0 (GPIO20/21),
// which is not connected on v2.3.3. Use SQC485I_CONSOLE instead.
#if ARDUINO_USB_CDC_ON_BOOT
#define SQC485I_CONSOLE Serial
#else
inline HWCDC &sqc485iUsbSerial()
{
  static HWCDC usbSerial;
  return usbSerial;
}
#define SQC485I_CONSOLE sqc485iUsbSerial()
#endif

#else // SQC485I_V1
// v1 has a CH340K USB-UART bridge on UART0 (GPIO20/21) = `Serial`.
#define SQC485I_CONSOLE Serial
#endif

// UART used for the RS485 transceiver
#define SQC485I_RS485_SERIAL Serial1

class SQC_485I
{
public:
#if defined(SQC485I_V2_3_3)
  static constexpr const char *BOARD_NAME = "SQC485I v2.3.3";
  // GPIO2 drives the RS485 /RE pin directly and the white LED through an
  // inverter: GPIO2 LOW = receiver ON and LED ON.
  static constexpr int8_t LED_PIN = 2;
  static constexpr int8_t RS485_RO = 0;  // Receiver Output  (GPIO0)
  static constexpr int8_t RS485_DI = 1;  // Driver Input     (GPIO1)
  static constexpr int8_t RS485_DE = 9;  // GPIO9 -> SN74LVC2G04 inverter -> DE
  static constexpr int8_t RS485_RE = 2;  // /RE, shared with LED
  static constexpr uint8_t DE_TX_LEVEL = LOW;   // inverted: LOW = driver ON
  static constexpr uint8_t DE_RX_LEVEL = HIGH;
  static constexpr bool HAS_EXTERNAL_XTAL = false;
#else
  static constexpr const char *BOARD_NAME = "SQC485I v1";
  // GPIO2 LOW turns on Q1 (AO3401A): Vext output + LED1.
  static constexpr int8_t LED_PIN = 2;
  static constexpr int8_t RS485_RO = 19; // Receiver Output  (GPIO19)
  static constexpr int8_t RS485_DI = 18; // Driver Input     (GPIO18)
  static constexpr int8_t RS485_DE = 9;  // DE and /RE tied together
  static constexpr int8_t RS485_RE = -1; // no separate /RE pin
  static constexpr uint8_t DE_TX_LEVEL = HIGH;
  static constexpr uint8_t DE_RX_LEVEL = LOW;
  static constexpr bool HAS_EXTERNAL_XTAL = true;
#endif

  SQC_485I() {}

  // Initialise the HT-CT62 module (Heltec Mcu.begin).
  // v1:     useExternalXtal selects the 32.768 kHz crystal on GPIO0/1
  //         (fitted on v1 boards; pass false if your board has none).
  // v2.3.3: there is no crystal and GPIO0/1 carry RS485, so the argument
  //         is ignored and the internal RC oscillator is always used.
  void begin(uint32_t baud = 9600, bool useExternalXtal = HAS_EXTERNAL_XTAL)
  {
    (void)baud; // kept for v1 API compatibility; use rs485Begin() for RS485
#if defined(SQC485I_V2_3_3)
    useExternalXtal = false;
#endif
    Mcu.begin(HELTEC_BOARD, useExternalXtal);
  }

  // LED control.
  // v1:     the LED is on the Vext power switch; ledOn() also enables Vext.
  // v2.3.3: the LED shares GPIO2 with RS485 /RE. ledOn() = receiver ON,
  //         ledOff() = receiver OFF (RS485 reception stops while off).
  void ledOn()
  {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
  }
  void ledOff()
  {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
  }

  // ---------------- RS485 ----------------

  // Start the RS485 UART and leave the transceiver in receive mode.
  void rs485Begin(uint32_t baud = 9600, uint32_t config = SERIAL_8N1)
  {
    pinMode(RS485_DE, OUTPUT);
    rs485ReceiveMode();
    SQC485I_RS485_SERIAL.begin(baud, config, RS485_RO, RS485_DI);
  }

  // Driver ON. On v2.3.3 the receiver is switched off while transmitting,
  // so the board does not read back its own frame.
  void rs485TransmitMode()
  {
#if defined(SQC485I_V2_3_3)
    pinMode(RS485_RE, OUTPUT);
    digitalWrite(RS485_RE, HIGH); // receiver OFF (LED off while sending)
#endif
    digitalWrite(RS485_DE, DE_TX_LEVEL);
    delayMicroseconds(switchDelayUs);
  }

  // Driver OFF, receiver ON.
  void rs485ReceiveMode()
  {
    digitalWrite(RS485_DE, DE_RX_LEVEL);
#if defined(SQC485I_V2_3_3)
    pinMode(RS485_RE, OUTPUT);
    digitalWrite(RS485_RE, LOW); // receiver ON (LED on)
#endif
  }

  // Send a buffer and return to receive mode once the last bit is out.
  size_t rs485Send(const uint8_t *data, size_t len)
  {
    rs485TransmitMode();
    size_t n = SQC485I_RS485_SERIAL.write(data, len);
    SQC485I_RS485_SERIAL.flush(); // wait until the UART FIFO is empty
    rs485ReceiveMode();
    return n;
  }
  size_t rs485Send(const String &message)
  {
    return rs485Send((const uint8_t *)message.c_str(), message.length());
  }

  int rs485Available() { return SQC485I_RS485_SERIAL.available(); }
  int rs485Read() { return SQC485I_RS485_SERIAL.read(); }

  // Read everything currently arriving, until the line is idle for
  // idleTimeoutMs. Returns "" if nothing was received.
  String rs485ReceiveString(uint32_t idleTimeoutMs = 50)
  {
    String buffer;
    if (!SQC485I_RS485_SERIAL.available())
      return buffer;
    uint32_t last = millis();
    while (millis() - last < idleTimeoutMs)
    {
      while (SQC485I_RS485_SERIAL.available())
      {
        buffer += (char)SQC485I_RS485_SERIAL.read();
        last = millis();
      }
      delay(1);
    }
    return buffer;
  }

  // Delay after enabling the driver before the first byte (microseconds).
  uint32_t switchDelayUs = 100;
};

#endif
