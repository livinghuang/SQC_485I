# SQC_485I Arduino 函式庫

本函式庫是為客製化開發板 **SQC_485I** 所設計，該板基於 **Heltec Wireless Mini（HT-CT62）** 模組，整合了 RS485 通訊、Vext 控制 LED，以及官方 LoRaWAN SDK 初始化流程。

## 📦 功能特色

- 使用 Heltec 官方 LoRaWAN 函式庫（版本 3.0.2）初始化無線模組
- 控制 Vext 外部電源（GPIO2，低電位啟動 AO3401）
- 整合 RS485 通訊（MAX3485ESA）：
  - `DE`: GPIO2
  - `DI`: GPIO18
  - `RO`: GPIO19
- 提供簡單易用的函式來傳送與接收 RS485 資料

## 📚 相依函式庫

本函式庫依賴 Heltec 官方 ESP32 LoRa 函式庫：

- [Heltec ESP32 Library v3.0.2](https://github.com/HelTecAutomation/Heltec_ESP32)

請透過 Arduino Library Manager 或 GitHub 安裝此函式庫。

## 🛠️ 板子資訊

- **核心模組**：Heltec Wireless Mini（HT-CT62，ESP32-C3）
- **RS485 收發器**：MAX3485ESA
- **外部 32.768kHz 晶振**：用於 Deep Sleep 和 LoRa 定時精度
- **Vext/LED 控制**：透過 GPIO2 控制 AO3401 開關
- **用戶按鍵**：GPIO9

## 🧪 測試環境

- ✅ 測試平台：
  - macOS Apple Silicon（ARM 架構 Mac）
  - Arduino IDE 2.x
  - Heltec ESP32 開發板包版本 3.0.2
- 選擇開發板：**"Wireless Stick Lite (ESP32-C3)"** 或相容模組

## 🚀 使用方式

### 安裝方式

1. 將此專案下載（或複製）到 Arduino 函式庫目錄：
```

\~/Documents/Arduino/libraries/SQC\_485I\_Library/

````

### 基本範例

```cpp
#include <SQC_485I.h>

SQC_485I board;

void setup() {
board.begin();         // 初始化模組
board.ledOn();         // 開啟 LED（Vext）
delay(1000);
board.ledOff();

board.sendRS485("Hello from SQC_485I\n");
}

void loop() {
String rx = board.receiveRS485();
if (rx.length()) {
 Serial.println("接收資料：" + rx);
}
delay(200);
}
````

## 📄 授權

MIT 授權。可自由使用、修改與散佈。

---

由 Siliq 開發，2025 年。
