# AegisOne-Genesys-Hackathon
Industrial IoT Safety Node - Genesys 2.0 Hackathon

## ⚠️ The Problem
Industrial equipment failures and environmental hazards (toxic gas leaks, fire outbreaks) cost lives and billions in unplanned downtime. Current SCADA systems are too expensive for SMEs, and cloud-dependent alarms have too much latency to stop high-speed machinery during an emergency.

## ⚙️ Our Solution
Aegis One is an autonomous, edge-first safety node that retrofits onto legacy machinery. It continuously monitors environmental air quality and mechanical vibrations. 

## 🚀 System Workflow
1. **Edge Processing:** The ESP32 processes sensor data locally, ensuring protection even if Wi-Fi fails.
2. **Instant Actuation:** If critical vibration or toxic gas is detected, a hardware interrupt triggers a 5V Relay, physically cutting power to the machine in <10ms to prevent fires or explosions.
3. **Cloud Analytics & Alerts:** Real-time data is visualized on a Blynk Dashboard, logged to ThingSpeak for predictive maintenance, and emergency alerts are pushed instantly via Telegram.

## 🌍 Green Computing Impact
By detecting toxic leaks before they saturate the air and shutting down failing machinery before it causes industrial fires, Aegis One actively prevents localized environmental disasters and significantly reduces the carbon footprint associated with replacing destroyed industrial equipment.
