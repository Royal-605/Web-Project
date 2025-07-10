🔋 Prepaid Energy Meter (WattPay)

A smart prepaid electricity monitoring and management system using ESP32, PZEM-004T, and Adafruit IO.




📖DESCRIPTION:


The Prepaid Energy Meter is an IoT-based solution designed to monitor and manage electricity consumption. It lets users recharge electricity balance before usage, set limits, and monitor real-time usage. When the balance runs out, the power supply is automatically disconnected to avoid overuse.



🚀 FEATURES:


🔌 Real-time monitoring of voltage, current, and energy.

💰 Recharge system using Razorpay and Adafruit IO.

📟 16x2 LCD display for live updates.

🌐 Web dashboard for remote monitoring.

✉️ Telegram notifications for recharge confirmations and low balance warnings.

⚡ Automatic power cutoff using a relay module when balance = 0.

🔄 Live updates to balance via MQTT from Adafruit IO.

🌍 Web server hosted on ESP32 for local monitoring.




🛠️ HARDWARE COMPONENTS:


ESP32 Dev Board

PZEM-004T V3.0 (Voltage & Current Sensor)

16x2 LCD Display with I2C module

Relay Module

LED Bulb (Load Simulation)

HLK-PM03 Power Supply Module

Jumper Wires

Breadboard/PCB




🧠 SOFTWARE STACK:


Arduino IDE (for ESP32 programming)

Adafruit IO (MQTT-based IoT platform)

Razorpay (Payment Gateway)

Pipedream (Webhook intermediary)

Telegram Bot API (Notifications)





 
🧾 HOW IT WORKS:


User recharges using Razorpay.

Payment is received and processed via Pipedream, which sends the updated balance to Adafruit IO.

ESP32 fetches the balance from Adafruit IO.

Power is supplied only when there is a positive balance.

Real-time values are shown on LCD and Web Dashboard.

On low balance, a Telegram alert is sent.

If balance hits zero, ESP32 cuts off the load via relay.

🧠 Code

The complete ESP32 Arduino code is available in the repository under prepaid_energy_meter.ino. It includes:

WiFi & MQTT setup

PZEM-004T interfacing

LCD & Web dashboard updates

Razorpay-Adafruit IO communication

Telegram notification integration

Smart billing logic

🖥️ Web Dashboard

The ESP32 hosts a sleek local webpage showing:

Voltage

Current

Remaining Balance

Energy Usage

🔐 Security Note

Important: For security reasons, do not commit sensitive credentials (WiFi passwords, API keys, Telegram tokens) to public repositories.

📦 Future Improvements

Mobile app integration

Real-time recharge from app

Automatic tariff adjustments

Cloud-based data logging
