# Fire-Warning-System
An ESP8266-based tactical radar for Search and Rescue robotics. Features autonomous 180° sweeping, IR fire detection, ultrasonic distance mapping, and an offline, real-time AJAX web dashboard.

# 🚒 ResQBot: Tactical Fire & Distance Radar (ESP8266)

## 📖 Overview
The **Tactical Fire & Distance Radar** is a subsystem designed for search and rescue (SAR) robotics. Powered by an ESP8266, it acts as an autonomous scanning module. It uses a servo motor to constantly sweep an ultrasonic sensor and an IR flame sensor back and forth. 

When a fire is detected, the radar instantly halts its sweep, locks onto the exact trajectory of the hazard, and pushes an emergency alert to a localized, offline web dashboard.

## ✨ Features
* **100% Offline Capability:** The ESP8266 hosts its own Wi-Fi Access Point (AP), meaning this system works perfectly in disaster zones with no internet infrastructure.
* **Asynchronous AJAX Dashboard:** The web UI updates in real-time without ever needing to refresh the page, keeping the connection stable and the animations smooth.
* **Animated Tactical HUD:** Features a spinning radar graphic built with CSS conic-gradients that turns into a pulsing red laser-lock when a hazard is detected.
* **Auto-Resume Logic:** The radar locks onto the fire until it is extinguished, after which it automatically resumes its sector sweep.

## 🛠️ Hardware Requirements
* 1x **ESP8266 Microcontroller** (NodeMCU 1.0 or Wemos D1 Mini)
* 1x **SG90 Micro Servo Motor**
* 1x **HC-SR04 Ultrasonic Distance Sensor**
* 1x **IR Flame Sensor Module** (Active-Low)
* Jumper Wires & Breadboard

## 🔌 Wiring & Pinout Guide
To prevent memory crashes and timer conflicts on the ESP8266, strictly adhere to this pinout:

| Component | Pin Name | ESP8266 Pin |
| :--- | :--- | :--- |
| **Servo Motor** | Signal (Orange/Yellow) | `D1` (GPIO 5) |
| **IR Flame Sensor** | D0 / Digital Out | `D2` (GPIO 4) |
| **Ultrasonic Sensor** | TRIG | `D5` (GPIO 14) |
| **Ultrasonic Sensor** | ECHO | `D6` (GPIO 12) |

*(Note: Ensure all components share a common Ground (GND) with the ESP8266).*

## 🚀 Installation & Usage

### 1. Flash the Code
1. Open the `.ino` file in the Arduino IDE.
2. Ensure you have the **ESP8266 Board Package** installed via the Boards Manager.
3. Select **NodeMCU 1.0 (ESP-12E Module)**.
4. Upload the code to your board.

### 2. Connect to the Radar
1. Once powered on, open your smartphone or laptop Wi-Fi settings.
2. Connect to the network: **`ResQBot_Radar`** (Password: `rescueadmin`).
3. Open a web browser (Chrome/Safari) and navigate to **`http://192.168.4.1`**.

### 3. Operation
* The dashboard will display a spinning green radar and the real-time distance to objects in front of it.
* Introduce a flame near the IR sensor. The dashboard will instantly turn red, trigger a `*** FIRE DETECTED ***` warning, and display the exact locked angle of the hazard.

## 💻 Under the Hood (Code Architecture)
* **Backend:** C++ utilizing standard `<Servo.h>`, `<ESP8266WiFi.h>`, and `<ESP8266WebServer.h>`.
* **Frontend:** A raw string literal containing HTML, CSS (keyframes, flexbox, radial/conic gradients), and JavaScript (Fetch API for AJAX polling every 500ms).
* **Non-Blocking Logic:** The main `loop()` relies entirely on `millis()` timers, ensuring the servo movement and sonar pings never interrupt the web server handling.
