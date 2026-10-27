# SQC\_485I Arduino 函式庫

[English](README.en.md)

SQC\_485I 是 RS485 ↔ LoRa / LoRaWAN 轉換板，核心是 Heltec HT-CT62 模組（ESP32-C3 + SX1262）。本函式庫提供板子初始化、LED 控制與 RS485 收發方向控制，支援兩個硬體版本：

| 版本 | 狀態 | 在程式裡怎麼選 |
|---|---|---|
| **v2.3.3** | 目前販售中 | `#define SQC485I_V2_3_3` |
| **v1** | 舊版 | `#define SQC485I_V1`（或不寫，見下方） |

---

## 1. 先確認手上的板子是哪一版

用 USB 線接上電腦，看出現的序列埠：

| | v1 | v2.3.3 |
|---|---|---|
| USB 介面 | CH340K USB 轉 UART 晶片 | ESP32-C3 內建 USB（板上沒有 CH340） |
| macOS | `/dev/cu.wchusbserial*` 或 `/dev/cu.usbserial*` | `/dev/cu.usbmodem*` |
| Windows 裝置管理員 | `USB-SERIAL CH340 (COMx)` | `USB 序列裝置 (COMx)`／`USB JTAG/serial debug unit` |
| Linux | `/dev/ttyUSB*` | `/dev/ttyACM*` |
| 板上元件 | 有 32.768 kHz 晶振、RS485 晶片 MAX3485 | RS485 晶片 SN65HVD1780、有鋰電池接頭與充電 LED |

不確定的話，對照本 repo 的電路圖：
- v1：[`Schematic_SQC485IS_2025-07-02.pdf`](Schematic_SQC485IS_2025-07-02.pdf)
- v2.3.3：[`Schematic_SQC485IS_v2.3.3_2026-06-13.pdf`](Schematic_SQC485IS_v2.3.3_2026-06-13.pdf)

---

## 2. 安裝

### 開發環境（已驗證的版本）

| 項目 | 版本 |
|---|---|
| Arduino IDE | 2.x（或 arduino-cli 1.x） |
| 板子套件 Heltec ESP32 Series | **3.3.8** |
| 函式庫 Heltec ESP32 Dev-Boards | **2.1.5** |

> ⚠️ Heltec ESP32 Dev-Boards **2.1.6** 在 ESP32-C3 板子上編譯會失敗（`'Wire1' was not declared`），連 Heltec 自己的範例也一樣。請在函式庫管理員裡選 **2.1.5**。

1. Arduino IDE → 喜好設定 → 額外的開發板管理員網址，加入：
   `https://resource.heltec.cn/download/package_heltec_esp32_index.json`
2. 開發板管理員安裝 **Heltec ESP32 Series**。
3. 函式庫管理員安裝 **Heltec ESP32 Dev-Boards 2.1.5**。
4. 把本 repo 下載成 ZIP，用「草稿碼 → 匯入程式庫 → 加入 .ZIP 程式庫」安裝；或直接複製到 `~/Documents/Arduino/libraries/SQC_485I/`。
5. 工具 → 開發板 → Heltec ESP32 Series → **Heltec Wireless Mini Shell**（v1 與 v2.3.3 都選這個）。
6. LoRaWAN 範例另外要在「工具 → LoRaWan Region」選你的頻段（台灣用 `REGION_AS923`）。

---

## 3. 在程式裡選版本

在 `#include <SQC_485I.h>` **之前**寫一行：

```cpp
#define SQC485I_V2_3_3   // v2.3.3 板子
// #define SQC485I_V1    // v1 板子
#include <SQC_485I.h>
```

- **兩個都沒寫時，預設是 v1**，讓舊的 v1 程式不用改就能編譯。編譯時會出現一則提示：`SQC_485I: no board revision defined, using SQC485I_V1`。
- **v2.3.3 的板子一定要寫 `#define SQC485I_V2_3_3`**，選錯版本 RS485 會完全不通（腳位與 DE 極性都不同）。
- 兩個同時寫會編譯錯誤。
- 所有範例第一行預設是 `SQC485I_V2_3_3`；v1 的使用者把那一行改成 `#define SQC485I_V1`。

---

## 4. 腳位對照

| 功能 | v1 | v2.3.3 |
|---|---|---|
| RS485 RO（接收輸出 → MCU RX） | GPIO19 | **GPIO0** |
| RS485 DI（MCU TX → 驅動輸入） | GPIO18 | **GPIO1** |
| RS485 DE | GPIO9，HIGH = 送出 | GPIO9 → 反相器 → DE，**LOW = 送出** |
| RS485 /RE | 與 DE 接在一起 | **GPIO2**，LOW = 接收開啟 |
| LED | GPIO2，LOW = 亮（同時開啟 Vext 電源輸出） | GPIO2，LOW = 白色 LED 亮（與 /RE 共用） |
| 使用者按鍵 | GPIO9 | GPIO9 |
| USB | CH340K ↔ UART0（GPIO20/21） | ESP32-C3 內建 USB（GPIO18/19） |
| 32.768 kHz 晶振 | GPIO0/1 | 無 |
| LoRa SX1262（模組內部） | NSS 8、SCK 10、MOSI 7、MISO 6、RST 5、BUSY 4、DIO1 3 | 相同 |
| RS485 終端電阻 120 Ω | 板上有 | **板上沒有**，在匯流排末端時請自行外加 |
| RS485 收發器 | MAX3485ESA | SN65HVD1780DR |
| 電源 | 8–30 V DC 或 USB 5 V | 8–30 V DC、USB 5 V 或鋰電池（板上有充電電路） |

### v2.3.3 要注意的地方

- **偵錯訊息要用 `SQC485I_CONSOLE`，不要用 `Serial`。** Heltec Wireless Mini Shell 的板子設定沒有開「USB CDC On Boot」，`Serial` 會輸出到 GPIO20/21，而 v2.3.3 的 USB 接的是 ESP32-C3 內建 USB。`SQC485I_CONSOLE` 在 v2.3.3 指向內建 USB，在 v1 就是 `Serial`。
- **LED 和 RS485 /RE 共用 GPIO2。** LED 亮代表 RS485 接收開著；`ledOff()` 會關掉接收。送資料時函式庫會暫時關掉接收（避免讀回自己送出的資料），所以 LED 會短暫熄滅。
- **GPIO9 同時是使用者按鍵和 DE 控制。** 按住按鍵時 RS485 驅動器會被強制打開；開機或重置時按住按鍵會進入燒錄模式。
- 板上沒有 32.768 kHz 晶振，`board.begin()` 在 v2.3.3 一律使用內部振盪器。

### v1 要注意的地方

- GPIO9 同時用於 RS485 DE 與 USB 自動燒錄電路。燒錄完如果 RS485 不通，請拔插一次 USB。
- `board.begin()` 預設使用外部 32.768 kHz 晶振（與舊版函式庫相同）。若你的 v1 板子沒有晶振，請用 `board.begin(9600, false)`。

---

## 5. API

```cpp
SQC_485I board;

board.begin();                         // 初始化 HT-CT62（Heltec Mcu.begin）
board.ledOn();  board.ledOff();

board.rs485Begin(9600, SERIAL_8N1);    // 啟動 RS485（Serial1），預設為接收狀態
board.rs485Send("hello");              // 送出字串，送完自動切回接收
board.rs485Send(buf, len);             // 送出位元組
String s = board.rs485ReceiveString(); // 讀取到匯流排閒置為止，沒有資料回傳 ""
board.rs485Available(); board.rs485Read();
board.rs485TransmitMode(); board.rs485ReceiveMode(); // 需要自己控制方向時使用

SQC_485I::BOARD_NAME   // "SQC485I v2.3.3" 或 "SQC485I v1"
SQC_485I::RS485_RO / RS485_DI / RS485_DE / RS485_RE / LED_PIN
SQC485I_CONSOLE        // USB 偵錯序列埠
```

---

## 6. 範例

| 範例 | 說明 |
|---|---|
| `LED_TOGGLE` | LED 每秒閃爍 |
| `RS485` | RS485 回音：收到什麼就送回什麼 |
| `LoRaBasic485/Rs485ToLora` | RS485 收到的資料用 LoRa（點對點）送出 |
| `LoRaBasic485/LoraToRs485` | LoRa（點對點）收到的資料轉送到 RS485 |
| `LoRaWAN485/Rs485ToLoraWAN` | RS485 收到的資料用 LoRaWAN 上行送出 |
| `LoRaWAN485/LoraWANToRs485` | LoRaWAN 下行資料轉送到 RS485 |

LoRaWAN 範例裡的金鑰全部是 0，請換成你的 LoRaWAN 網路伺服器配發的值。LoRa 的頻率與發射功率請符合你所在國家的法規。

---

## 授權

MIT License，見 [LICENSE](LICENSE)。
