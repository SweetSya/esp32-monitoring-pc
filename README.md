# 💻 ESP32 PC Monitoring Dashboard

An **ESP32-based hardware monitor** that displays real-time **CPU, GPU, RAM usage, and temperatures** on a small TFT screen.  
The project communicates with a **Python script running on the PC** (from a companion repo) which collects system metrics and sends them to the ESP32 via serial/Bluetooth.

---

## 🚀 Features

- 📊 **Real-time monitoring** of:
  - CPU usage & temperature
  - GPU usage & temperature
- 🖥️ **TFT Display Output** using ESP32 (ILI9341 / ILI9488 / XPT2046 supported)
- 🔗 **Data transfer** via Serial / Bluetooth from PC to ESP32
- ⚡ Lightweight, responsive, and always-on dashboard
- 🔧 Configurable update intervals and screen layout

---

## 🛠️ Tech Stack

- **Hardware:** ESP32 Development Board + TFT Display (ILI9341 / ILI9488)  
- **ESP32 Firmware:** Arduino (C++) + [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) library  
- **PC Side:** Python 3 script (separate repo) that collects system metrics using `psutil` / OpenHardwareMonitor and sends structured data  
- **Protocols:** Serial (USB) or Bluetooth communication  

---
## 📷 Preview

![alt text](https://github.com/SweetSya/esp32-monitoring-pc/blob/main/preview.jpeg?raw=true)
