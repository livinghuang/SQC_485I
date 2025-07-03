# SQC\_485I Arduino 函式庫

本函式庫是為客製化開發板 **SQC\_485I** 所設計。此板基於 **Heltec Wireless Mini（HT-CT62）** 模組，整合 RS485 通訊、LED 電源控制（Vext）、以及 Heltec 官方 LoRaWAN SDK 初始化流程。

---

## 功能簡介

* 使用 Heltec LoRaWAN 函式庫（v3.0.2）初始化無線模組
* 控制 Vext 電源（GPIO2，透過 AO3401 控制 LED）
* 整合 RS485 通訊（使用 MAX3485ESA）：

  * `DE`: GPIO2
  * `DI`: GPIO18
  * `RO`: GPIO19
* 提供簡易 API 傳送與接收 RS485 資料

---

## 相依函式庫

此函式庫依賴 [Heltec ESP32 LoRa 函式庫 v3.0.2](https://github.com/HelTecAutomation/Heltec_ESP32)。

請透過 Arduino Library Manager 或手動安裝來完成依賴安裝。

---

## 板子硬體資訊

| 項目           | 說明                                      |
| ------------ | --------------------------------------- |
| 模組           | Heltec Wireless Mini（HT-CT62, ESP32-C3） |
| RS485 晶片     | MAX3485ESA                              |
| 32.768kHz 晶振 | 用於 LoRa RX delay 與 Deep Sleep 精準時序      |
| Vext 控制      | GPIO2 控制 AO3401 開關                      |
| 用戶按鍵         | GPIO9                                   |
| RS485 傳輸方向   | GPIO9                                   |
| USB 開發模式     | GPIO9                                   |

\::: warning 注意事項
由於 **GPIO9** 同時用於 RS485 DE 控制與 USB 燒錄模式，
每次重新燒錄程式後，GPIO9 的電平可能會被上鎖，導致 RS485 通訊方向控制失效。
**請在燒錄完成後，重新插拔 USB 連接線以恢復 GPIO9 控制功能。**
\:::

---

## 開發與測試環境

* macOS (Apple Silicon, ARM 架構)
* Arduino IDE 2.x
* Heltec ESP32 Board Support Package v3.0.2
* 測試板型：**Wireless Stick Lite (ESP32-C3)**

---

## 安裝方式

1. 將此函式庫下載或複製到 Arduino 函式庫目錄中：

```
~/Documents/Arduino/libraries/SQC_485I_Library/
```

2. 重新啟動 Arduino IDE，並從「範例」中選擇 `SQC_485I` 測試範例進行開發。

---

## 授權

本專案採用 MIT 授權，歡迎自由使用、修改與散佈。

---

由 [Siliq](https://github.com/livinghuang) 開發，2025 年。

