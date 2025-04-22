# 🚨 Smart Streetlight System for Rural Roads (ESP32 + ThingSpeak)

This project presents a **low-cost smart streetlight solution** designed for rural areas with limited infrastructure. It uses an **ESP32 microcontroller**, **light sensors (LDRs)**, and **ThingSpeak cloud** to control a streetlight that turns on only **at night** and **when a vehicle passes by**.

---

## 🌟 Features

- 🔦 Streetlight control based on **ambient light** and **car headlights** detection
- 🧠 Multithreaded operation using **ESP32 dual-core** (LED logic on Core 1, cloud communication on Core 0)
- ☁️ Real-time data upload to **ThingSpeak**
- 📡 Adjustable light-on time read from **ThingSpeak field 3**
- 🌓 Sends day/night status to **field 1**
- 🚗 Sends car detection log to **field 2** (after 20s delay to avoid API flooding)
- 📟 Clear Serial Monitor messages with sensor readings and state

---

## ⚙️ Hardware Components

| Component           | Quantity | Description                                |
|---------------------|----------|--------------------------------------------|
| ESP32 Dev Module    | 1        | Microcontroller with dual-core support     |
| LDR (photoresistor) | 2        | One for ambient light, one for car detection |
| Resistors (10kΩ)    | 2        | Voltage dividers for LDRs                  |
| LED (White)         | 1        | Simulated streetlight                      |

---

## 🔌 Pin Configuration

| Function              | ESP32 Pin |
|-----------------------|-----------|
| Ambient Light Sensor  | GPIO 35   |
| Car Detection Sensor  | GPIO 32   |
| Streetlight (LED)     | GPIO 4    |

---

## ☁️ ThingSpeak Fields

| Field | Purpose                       |
|-------|-------------------------------|
| 1     | Day (1) / Night (0) status    |
| 2     | Sends 1 when car is detected  |
| 3     | Light on-time in seconds      |

> ⚠️ Note: ThingSpeak limits update frequency to ~15s. This project respects this by only sending car detections 20s after the LED turns on.

---

## 🔁 System Logic

- During the **day**, the streetlight remains OFF.
- At **night**, the system waits for **car headlights** to be detected.
- When detected:
  - The **LED turns ON**
  - A **20-second timer** starts to send a signal to ThingSpeak (Field 2)
- If the car remains near the sensor:
  - The LED **stays ON** and the delay timer **resets**
- The **off-time is configurable** via Field 3 in the ThingSpeak channel.

---

## 🧪 Serial Monitor Output

This section shows examples of how the Serial Monitor displays sensor values and system status in real time:

#### ☀️ During the Day
When ambient light is high (above the `nightThreshold`), the system recognizes it is daytime. The LED remains off, and car detection is ignored:
Ambient Sensor: 1120 | Car Sensor: 850 | Day Ambient Sensor: 1090 | Car Sensor: 950 | Day

#### 🌙 During the Night Without a Car
When ambient light is low but no car headlights are detected:
Ambient Sensor: 820 | Car Sensor: 740 | Night | No Car Ambient Sensor: 790 | Car Sensor: 820 | Night | No Car

#### 🌙 During the Night With a Car
If it is night and the car’s headlights are detected (car sensor value exceeds the threshold):
Ambient Sensor: 780 | Car Sensor: 1120 | Night | Car Detected LED turned ON - waiting 20s to send to field 2...

#### 🚘 Car Stays Near the Sensor
If the car remains in place and its headlights continue to be detected after the initial delay:
Car still detected — keeping LED ON.


#### 🌒 Car Leaves After Timeout
When the car moves away, and the timeout is reached:
LED turned OFF — no car detected.

## 🤝 Credits
Developed by Group 12, as part of a project for intelligent and sustainable systems using IoT and embedded platforms.

List of authors:
ALLISSON SILVA RAYMUNDO, CARLOS AUGUSTO DAVID NETO, DOUGLAS ALESSANDRO DE ALCANTARA, FELIPE BELLO GRIGOLATO, GABRIEL AUGUSTO DAVID, GABRIEL MISTRONI RAMOS SOUZA, GUSTAVO HENRIQUE LOPES CARDOSO, JHUAN CAMARGO, THAIS DE MORAES SOUZA & VITOR TOLEDO.
