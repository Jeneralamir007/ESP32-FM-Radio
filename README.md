# ESP32-FM-Radio
ESP32 FM Radio with RDA5807M, LCD, Rotary Encoder and Station Memory

 # ESP32 FM Radio

A complete FM radio built with ESP32 and RDA5807M.

## Features

- FM Radio
- Auto Scan
- Save Station
- Load Station
- Replace Station
- LCD 16x2
- Rotary Encoder
- Boot Screen

## Hardware

- ESP32
- RDA5807M
- PAM8403
- LCD1602 I2C
- KY-040 Encoder

## Pin Connections

| ESP32 Pin |       Component         |
|-----------|-------------------------|
| GPIO21    | SDA (RDA5807 & LCD I2C) |
| GPIO22    | SCL (RDA5807 & LCD I2C) |
| GPIO32    | Rotary Encoder CLK      |
| GPIO33    | Rotary Encoder DT       |
| GPIO25    | Rotary Encoder SW       |
| 3.3V      | RDA5807 VCC             |
| 5V        | LCD I2C VCC             |
| GND       | Common Ground           |

## Audio Connections

|     Module    |   Connection  |
|---------------|---------------|
| RDA5807 LOUT  | PAM8403 L IN  |
| RDA5807 ROUT  | PAM8403 R IN  |
| PAM8403 L OUT | Left Speaker  |
| PAM8403 R OUT | Right Speaker |

--------------------------------------------------------------------------------------------------------------------------------------------------------------------

you can change this text ("CYBER FM") to display your own name or logo during startup from hear

    lcd.setCursor(2,0);
    lcd.print("CYBER FM");      // Change this text ("CYBER FM") to display your own name or logo during startup.
