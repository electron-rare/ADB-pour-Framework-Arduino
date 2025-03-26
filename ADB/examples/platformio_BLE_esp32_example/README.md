# 🔌 ADB2BLE Adapter pour ESP32

[![Licence: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Plateforme: ESP32](https://img.shields.io/badge/Plateforme-ESP32-green.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-blue.svg)](https://www.arduino.cc/)
[![IDE: PlatformIO](https://img.shields.io/badge/IDE-PlatformIO-orange.svg)](https://platformio.org/)

> Ce projet permet de connecter des périphériques Apple Desktop Bus (ADB) comme les claviers et souris Apple vintage à des appareils modernes via Bluetooth Low Energy (BLE).

<p align="center">
  <img src="https://placeholder-image-url.com/adb2ble_adapter.jpg" alt="ADB2BLE Adapter" width="500">
</p>

## 📋 Sommaire
- [Fonctionnalités](#-fonctionnalités)
- [Prérequis](#-prérequis)
- [Schéma de connexion](#-schéma-de-connexion)
- [Installation](#-installation)
- [Utilisation](#-utilisation)
- [Dépannage](#-dépannage)
- [Détails techniques](#-détails-techniques)
- [Licence et auteur](#-licence-et-auteur)
- [Contributions](#-contributions)

## ✨ Fonctionnalités

- ✅ Conversion des signaux ADB vers BLE HID
- ⌨️ Support des claviers ADB
- 🖱️ Support des souris ADB
- 🔍 Reconnaissance automatique des périphériques
- 🔄 Reconnnexion automatique en cas de déconnexion
- ⚡ Faible latence (50Hz de taux de rafraîchissement)

## 🛠️ Prérequis

### Matériel
- ESP32 (testé sur ESP32-WROOM-32)
- Adaptateur de niveau logique 5V/3.3V ou simple résistance de 1kΩ
- Connecteur ADB (Mini-DIN 4 broches) ou adaptateur
- Câbles et connecteurs

### Logiciel
- PlatformIO
- Arduino IDE (version 2.x ou supérieure)
- Bibliothèques requises:
  - ADB (fournie dans ce dépôt)
  - Arduino BLE HID (intégrée à l'ESP32)

## 🔌 Schéma de connexion

┌─────────┐                                   ┌─────────┐
│         │                                   │         │
│  ESP32  │                                   │   ADB   │
│         │                                   │         │
├─────────┤                                   ├─────────┤
│  GPIO21 ├───[Résistance 1kΩ]────────────────┤  DATA   │
│         │                      │            │         │
│         │                      │            │         │
│         │              pull-up 4.7-10kΩ     │         │
│         │                      │            │         │
│   3.3V  ├───────────────────────────────────┤   +5V   │
│         │                                   │         │
│         │                                   │         │
│         │                                   │         │
│   GND   ├───────────────────────────────────┤   GND   │
└─────────┘                                   └─────────┘

⚠️ **Attention**: 
- Vérifiez la tolérance de votre ESP32 avant de connecter directement l'alimentation 5V d'ADB.
- **Important**: Une résistance de pull-up (entre 4.7kΩ et 10kΩ) doit être connectée entre la ligne DATA et +5V pour assurer le bon fonctionnement du bus ADB.

## 📥 Installation

1. Clonez ce dépôt:
   ```bash
   git clone https://github.com/electron_rare/stm32-adb2usb.git
   cd stm32-adb2usb
   ```

2. Ouvrez le projet dans PlatformIO IDE:
   - Sélectionnez "Open Project"
   - Naviguez jusqu'au dossier `stm32-adb2usb/lib/adb/examples/platformio_BLE_esp32_example`

3. Compilez et téléversez le projet sur votre ESP32:
   ```bash
   pio run --target upload
   ```

## 📝 Utilisation

1. Connectez vos périphériques ADB à l'adaptateur
2. Allumez l'ESP32
3. Recherchez le périphérique Bluetooth "ADB2BLE Adapter" depuis votre ordinateur ou appareil mobile
4. Connectez-vous à l'adaptateur
5. Utilisez vos périphériques ADB comme des périphériques Bluetooth standard

## ❓ Dépannage

| Problème | Solution |
|----------|----------|
| **Périphérique non détecté** | Vérifiez les connexions ADB et assurez-vous que les périphériques sont alimentés |
| **Mouvement de souris irrégulier** | Ajustez les valeurs de conversion de mouvement dans le code |
| **Touches de clavier incorrectes** | Consultez la correspondance des touches dans la bibliothèque ADB |

## 📊 Détails techniques

- **Taux de rafraîchissement**: 50Hz (configurable via `POLL_INTERVAL`)
- **Consommation électrique**: ~30-50mA en fonctionnement normal
- **Compatibilité**: La plupart des périphériques ADB standards

## 📄 Licence et auteur

Développé par **Clément SAILLANT** - *L'électron rare*

Copyright (C) 2025 Clément SAILLANT

Ce projet est sous licence [GNU GPL v3](https://www.gnu.org/licenses/gpl-3.0.html).

## 👥 Contributions

Les contributions sont les bienvenues! N'hésitez pas à:

- Ouvrir des [issues](https://github.com/electron_rare/stm32-adb2usb/issues)
- Soumettre des [pull requests](https://github.com/electron_rare/stm32-adb2usb/pulls)
- Partager vos retours d'expérience
