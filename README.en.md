# SQC\_485I Arduino Library

[中文](README.md)

SQC\_485I is an RS485 ↔ LoRa / LoRaWAN bridge board built around the Heltec HT-CT62 module (ESP32-C3 + SX1262). This library provides board initialisation, LED control and RS485 direction control for two hardware revisions:

| Revision | Status | How to select it |
|---|---|---|
| **v2.3.3** | Currently sold | `#define SQC485I_V2_3_3` |
| **v1** | Previous revision | `#define SQC485I_V1` (or nothing, see below) |

---

## 1. Which board do I have?

Plug in the USB cable and look at the serial port that appears:

| | v1 | v2.3.3 |
|---|---|---|
| USB interface | CH340K USB-UART bridge | ESP32-C3 native USB (no CH340 on the board) |
| macOS | `/dev/cu.wchusbserial*` or `/dev/cu.usbserial*` | `/dev/cu.usbmodem*` |
| Windows Device Manager | `USB-SERIAL CH340 (COMx)` | `USB Serial Device (COMx)` / `USB JTAG/serial debug unit` |
| Linux | `/dev/ttyUSB*` | `/dev/ttyACM*` |
| On the board | 32.768 kHz crystal, MAX3485 RS485 transceiver | SN65HVD1780 RS485 transceiver, Li-ion battery connector and charge LED |

If in doubt, compare with the schematics in this repository:
- v1: [`Schematic_SQC485IS_2025-07-02.pdf`](Schematic_SQC485IS_2025-07-02.pdf)
- v2.3.3: [`Schematic_SQC485IS_v2.3.3_2026-06-13.pdf`](Schematic_SQC485IS_v2.3.3_2026-06-13.pdf)

---

## 2. Installation

### Tested toolchain

| Item | Version |
|---|---|
| Arduino IDE | 2.x (or arduino-cli 1.x) |
| Board package Heltec ESP32 Series | **3.3.8** |
| Library Heltec ESP32 Dev-Boards | **2.1.5** |

> ⚠️ Heltec ESP32 Dev-Boards **2.1.6** fails to compile for ESP32-C3 boards (`'Wire1' was not declared`), including Heltec's own examples. Select **2.1.5** in the Library Manager.

1. Arduino IDE → Preferences → Additional boards manager URLs, add:
   `https://resource.heltec.cn/download/package_heltec_esp32_index.json`
2. Install **Heltec ESP32 Series** in the Boards Manager.
3. Install **Heltec ESP32 Dev-Boards 2.1.5** in the Library Manager.
4. Download this repository as a ZIP and use Sketch → Include Library → Add .ZIP Library, or copy it to `~/Documents/Arduino/libraries/SQC_485I/`.
5. Tools → Board → Heltec ESP32 Series → **Heltec Wireless Mini Shell** (for both v1 and v2.3.3).
6. For the LoRaWAN examples also choose your band in Tools → LoRaWan Region (e.g. `REGION_AS923`).

---

## 3. Selecting the revision in your sketch

Add one line **before** `#include <SQC_485I.h>`:

```cpp
#define SQC485I_V2_3_3   // v2.3.3 board
// #define SQC485I_V1    // v1 board
#include <SQC_485I.h>
```

- **If neither is defined, the library defaults to v1**, so existing v1 sketches compile unchanged. The build prints the note `SQC_485I: no board revision defined, using SQC485I_V1`.
- **A v2.3.3 board must use `#define SQC485I_V2_3_3`.** With the wrong revision RS485 does not work at all (different pins and DE polarity).
- Defining both is a compile error.
- Every example starts with `SQC485I_V2_3_3`; v1 users change that line to `#define SQC485I_V1`.

---

## 4. Pin map

| Function | v1 | v2.3.3 |
|---|---|---|
| RS485 RO (receiver out → MCU RX) | GPIO19 | **GPIO0** |
| RS485 DI (MCU TX → driver in) | GPIO18 | **GPIO1** |
| RS485 DE | GPIO9, HIGH = transmit | GPIO9 → inverter → DE, **LOW = transmit** |
| RS485 /RE | tied to DE | **GPIO2**, LOW = receiver on |
| LED | GPIO2, LOW = on (also switches the Vext output) | GPIO2, LOW = white LED on (shared with /RE) |
| User key | GPIO9 | GPIO9 |
| USB | CH340K ↔ UART0 (GPIO20/21) | ESP32-C3 native USB (GPIO18/19) |
| 32.768 kHz crystal | GPIO0/1 | none |
| LoRa SX1262 (inside the module) | NSS 8, SCK 10, MOSI 7, MISO 6, RST 5, BUSY 4, DIO1 3 | same |
| RS485 120 Ω termination | on board | **not on board**, add one externally at the end of the bus |
| RS485 transceiver | MAX3485ESA | SN65HVD1780DR |
| Power | 8–30 V DC or USB 5 V | 8–30 V DC, USB 5 V or Li-ion battery (on-board charger) |

### v2.3.3 notes

- **Print debug output with `SQC485I_CONSOLE`, not `Serial`.** The Heltec Wireless Mini Shell board definition builds with "USB CDC On Boot" disabled, so `Serial` goes to GPIO20/21, while the v2.3.3 USB port is the ESP32-C3 native USB. `SQC485I_CONSOLE` is the native USB on v2.3.3 and plain `Serial` on v1.
- **The LED and RS485 /RE share GPIO2.** LED on means the RS485 receiver is on; `ledOff()` turns reception off. While sending, the library turns the receiver off (so the board does not read back its own frame), so the LED blinks off briefly.
- **GPIO9 is both the user key and DE.** Holding the key forces the RS485 driver on; holding it during power-up or reset enters download mode.
- There is no 32.768 kHz crystal; on v2.3.3 `board.begin()` always uses the internal oscillator.

### v1 notes

- GPIO9 is shared by RS485 DE and the USB auto-download circuit. If RS485 does not work right after flashing, unplug and replug the USB cable.
- `board.begin()` uses the external 32.768 kHz crystal by default (same as earlier library versions). If your v1 board has no crystal, call `board.begin(9600, false)`.

---

## 5. API

```cpp
SQC_485I board;

board.begin();                         // initialise the HT-CT62 (Heltec Mcu.begin)
board.ledOn();  board.ledOff();

board.rs485Begin(9600, SERIAL_8N1);    // start RS485 on Serial1, receive mode
board.rs485Send("hello");              // send a string, then back to receive
board.rs485Send(buf, len);             // send bytes
String s = board.rs485ReceiveString(); // read until the bus is idle, "" if nothing
board.rs485Available(); board.rs485Read();
board.rs485TransmitMode(); board.rs485ReceiveMode(); // manual direction control

SQC_485I::BOARD_NAME   // "SQC485I v2.3.3" or "SQC485I v1"
SQC_485I::RS485_RO / RS485_DI / RS485_DE / RS485_RE / LED_PIN
SQC485I_CONSOLE        // USB debug serial port
```

---

## 6. Examples

| Example | What it does |
|---|---|
| `LED_TOGGLE` | Blinks the LED once per second |
| `RS485` | RS485 echo: sends back whatever it receives |
| `LoRaBasic485/Rs485ToLora` | Sends RS485 data over LoRa (point-to-point) |
| `LoRaBasic485/LoraToRs485` | Forwards LoRa (point-to-point) packets to RS485 |
| `LoRaWAN485/Rs485ToLoraWAN` | Sends RS485 data as LoRaWAN uplinks |
| `LoRaWAN485/LoraWANToRs485` | Forwards LoRaWAN downlinks to RS485 |

The keys in the LoRaWAN examples are all zeros; replace them with the values from your LoRaWAN network server. Use a LoRa frequency and output power that are legal in your country.

---

## License

MIT License, see [LICENSE](LICENSE).
