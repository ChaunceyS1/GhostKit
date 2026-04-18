# GhostKit 👻
> Physical Penetration Testing Toolkit for Flipper Zero

![Version](https://img.shields.io/badge/version-1.0.0-red)
![Platform](https://img.shields.io/badge/platform-Flipper%20Zero-orange)
![Firmware](https://img.shields.io/badge/firmware-Momentum-blue)
![Language](https://img.shields.io/badge/language-C-lightgrey)
![Status](https://img.shields.io/badge/status-In%20Development-yellow)
![GitHub](https://img.shields.io/badge/github-ChaunceyS1-black)

---

## ⚠️ Legal Disclaimer
GhostKit is designed for **authorized penetration testing and security research only.**
All modules must be used exclusively on systems and environments you own or have explicit written permission to test.
Unauthorized use is illegal under the CFAA and applicable state laws.
The author assumes no liability for misuse.

---

## What is GhostKit?

GhostKit is a custom Flipper Zero FAP (Flipper App Package) built from scratch in C on Momentum firmware.
It combines five physical penetration testing modules into a single on-device toolkit — designed to demonstrate real-world attack vectors during authorized security assessments.

Every module maps to MITRE ATT&CK techniques and generates CVSS-scored findings that export directly to a structured CSV report on the SD card.

---

## Modules

| Module | Description | Status | MITRE |
|---|---|---|---|
| GhostAccess | RFID/NFC access control bypass | 🔨 In Development | T1078, T1212 |
| GhostSignal | SubGHz RF signal capture & analysis | 🔨 In Development | T1040, T1056 |
| GhostDrop | BadUSB payload launcher | 🔨 In Development | T1566, T1059 |
| GhostBLE | Bluetooth device recon & mapping | 🔨 In Development | T1040, T1046 |
| GhostLog | On-device findings logger + CSV export | ✅ Complete | — |

---

## Demo

```
GhostKit Main Menu
├── GhostAccess   → RFID/NFC scanner
├── GhostSignal   → SubGHz RF scanner  
├── GhostDrop     → BadUSB launcher
├── GhostLog      → findings logger ✅
├── GhostBLE      → Bluetooth recon
└── About
```

---

## GhostLog Output

Every finding logged by GhostKit gets written to:
```
/ext/ghostkit/findings.csv
```

Format:
```
id,title,severity,mitre_technique
0,RFID Card Cloned,HIGH,T1078
1,Fixed Code RF Signal Captured,HIGH,T1040
2,BadUSB Payload Executed,CRITICAL,T1059
```

---

## Tech Stack

```
Language:     C
Firmware:     Momentum (Next-Flip)
Build Tool:   fbt (Flipper Build Tool)
Dev Env:      WSL2 + VS Code
Target:       Flipper Zero (STM32WB55)
```

---

## Project Structure

```
GhostKit/
  application.fam          ← app manifest
  ghostkit.c               ← main app + menu
  ghostkit.h               ← shared types + declarations
  modules/
    ghost_access.c         ← NFC/RFID module
    ghost_signal.c         ← SubGHz module
    ghost_drop.c           ← BadUSB module
    ghost_ble.c            ← Bluetooth module
    ghost_log.c            ← findings logger ✅
```

---

## Build Instructions

### Prerequisites
```bash
# WSL2 Ubuntu
sudo apt install -y git python3 python3-pip gcc \
  gcc-arm-none-eabi cmake ninja-build build-essential
```

### Clone and Build
```bash
# Clone Momentum firmware
git clone --recursive https://github.com/Next-Flip/Momentum-Firmware
cd Momentum-Firmware

# Clone GhostKit into external apps
git clone https://github.com/ChaunceyS1/GhostKit \
  applications/external/ghostkit

# Build GhostKit FAP
./fbt fap_ghostkit

# Deploy to Flipper over USB
./fbt launch APPSRC=applications/external/ghostkit
```

### Manual Install
```
1. Download ghostkit.fap from Releases
2. Copy to Flipper SD card: /apps/Tools/ghostkit.fap
3. On Flipper: Apps → Tools → GhostKit
```

---

## MITRE ATT&CK Coverage

| Technique ID | Name | Module |
|---|---|---|
| T1078 | Valid Accounts | GhostAccess |
| T1212 | Exploitation for Credential Access | GhostAccess |
| T1040 | Network Sniffing (RF adapted) | GhostSignal, GhostBLE |
| T1056 | Input Capture | GhostSignal |
| T1046 | Network Service Discovery | GhostBLE |
| T1566 | Phishing (HID adapted) | GhostDrop |
| T1059 | Command and Scripting Interpreter | GhostDrop |
| T1204 | User Execution | GhostDrop |

---

## Roadmap

- [x] App skeleton + main menu
- [x] GhostLog — findings logger with SD card export
- [x] GhostAccess — screen + navigation working
- [ ] GhostAccess — NFC card read, key recovery, clone, emulate
- [ ] GhostSignal — SubGHz passive scanner with protocol detection
- [ ] GhostDrop — BadUSB payload menu with payload library
- [ ] GhostBLE — BLE scanner + spam + remote control
- [ ] Custom app icon
- [ ] v1.0 release on GitHub

---

## About

Built by **Chauncey** — 18 year old sophomore studying Cybersecurity & Computer Science at Nova Southeastern University.

Building toward a career in penetration testing and red teaming.

- LinkedIn: (https://www.linkedin.com/in/chauncey-saintgermain-3a607b254/)
- GitHub: [github.com/ChaunceyS1](https://github.com/ChaunceyS1)

---

*GhostKit is an independent project not affiliated with Flipper Devices Inc. or the Momentum firmware team.*
