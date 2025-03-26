# Bibliothèque ADB pour Framework Arduino

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![Licence](https://img.shields.io/badge/licence-GNU%20GPL%20v3-green)
![État](https://img.shields.io/badge/état-Beta-orange)
![Framework](https://img.shields.io/badge/framework-Arduino-teal)

> **Note de développement** : Cette bibliothèque est implémentée pour le framework Arduino et actuellement optimisée principalement pour la plateforme STM32. Le support pour d'autres plateformes Arduino est en cours de développement.

## Vue d'ensemble

La bibliothèque ADB est une implémentation complète du protocole Apple Desktop Bus (ADB) en C++ pour le framework Arduino, fonctionnant sur diverses cartes comme STM32, ESP32, Arduino AVR et Teensy. Elle permet d'interfacer facilement des périphériques Apple vintage (claviers, souris) avec des systèmes modernes.

## Fonctionnalités clés

- 🔌 **Compatible avec plusieurs plateformes** : STM32 (primaire), Arduino, ESP32, Teensy (en cours de développement)
- 🧩 **API unifiée** : même interface de programmation quelle que soit la plateforme
- ⌨️ **Support complet des claviers ADB** : lecture des touches, gestion des modificateurs, contrôle des LEDs
- 🖱️ **Support complet des souris ADB** : lecture des mouvements et des boutons
- 🔄 **Conversion ADB vers HID** : mappage des codes de touches ADB vers les codes HID standard
- 📊 **Détection automatique des périphériques** : avec gestion des erreurs robuste
- 🛠️ **Utilitaires avancés** : fonctions d'aide au développement et débogage
- 📝 **Documentation complète** : commentaires explicites et exemples documentés

## Plateformes Arduino supportées

| Plateforme | Pin par défaut | Testé sur | Statut |
|------------|---------------|----------|--------|
| STM32 (Arduino Core) | PB4 | STM32F103, Bluepill, STM32F401, STM32F411 | ✅ Testé et stable |
| Arduino AVR | 2 | Uno, Mega | ⚠️ Support préliminaire |
| ESP32 (Arduino Core) | 21 | ESP32-WROOM, ESP32-WROVER | ⚠️ En développement |
| Teensy (Arduino Core) | 3 | Teensy 3.2, 4.0 | ⚠️ En développement |
| Autres cartes Arduino | 2 | Diverses | ⛔ Non testé |

## Installation

### Pour l'IDE Arduino
1. Téléchargez la bibliothèque depuis le dépôt
2. Extrayez les fichiers dans le dossier `libraries` de votre IDE Arduino
3. Redémarrez l'IDE Arduino
4. La bibliothèque est maintenant disponible dans "Croquis > Inclure une bibliothèque"

### Pour PlatformIO
1. Ajoutez la dépendance dans votre `platformio.ini`:
   ```ini
   lib_deps = 
     electronrare/ADB @ ^1.0.0
   ```
2. Ou installez-la manuellement dans le dossier `lib/` de votre projet

#### Configuration exemple pour STM32 (platformio.ini)
```ini
[env:bluepill_f103c8]
platform = ststm32
board = bluepill_f103c8
framework = arduino
lib_deps = 
    electronrare/ADB @ ^1.0.0
upload_protocol = stlink
monitor_speed = 115200
build_flags =
    -D PIO_FRAMEWORK_ARDUINO_ENABLE_CDC
    -D USBCON
    -D USB_MANUFACTURER="\"L'électron rare\""
    -D USB_PRODUCT="\"ADB2USB HID\""
```

#### Configuration exemple pour ESP32 (platformio.ini)
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
    electronrare/ADB @ ^1.0.0
monitor_speed = 115200
```

## Utilisation basique dans le framework Arduino

```cpp
#include <Arduino.h>
#include "adb.h"

// Configuration selon la carte Arduino
#if defined(ARDUINO_ARCH_STM32)
  #define ADB_PIN PB4
#else
  #define ADB_PIN 2  // Arduino, ESP32, etc.
#endif

// Initialisation des objets
ADB adb(ADB_PIN);
ADBDevices devices(adb);

void setup() {
  Serial.begin(115200);
  adb.init();
  Serial.println("ADB Initialized");
}

void loop() {
  // Lecture du clavier via Arduino
  bool error = false;
  auto keyPress = devices.keyboardReadKeyPress(&error);
  if (!error && keyPress.data.key0 != 0) {
    Serial.print("Key: 0x");
    Serial.println(keyPress.data.key0, HEX);
  }
  
  // Lecture de la souris
  error = false;
  auto mouseData = devices.mouseReadData(&error);
  if (!error) {
    int8_t xMove = adbMouseConvertAxis(mouseData.data.x_offset);
    int8_t yMove = adbMouseConvertAxis(mouseData.data.y_offset);
    // Traiter les données de mouvement...
  }
  
  delay(20);  // Intervalle de polling typique pour Arduino
}
```

## Exemples Arduino inclus

La bibliothèque est fournie avec plusieurs exemples pratiques pour Arduino IDE et PlatformIO :

- **basic_keyboard** : Utilisation simple d'un clavier ADB
- **basic_mouse** : Utilisation simple d'une souris ADB
- **multiplatform_basic** : Exemple multiplateforme (avec détection automatique)
- **keyboard_mouse_example** : Utilisation simultanée d'un clavier et d'une souris
- **multiplatform_event_handler** : Gestionnaire d'événements avancé
- **usb_hid_stm32** : Conversion ADB vers USB HID (STM32 uniquement)
- **multiplatform_device_info** : Scanner de périphériques ADB
- **platformio_stm32_example** : Exemple complet pour PlatformIO avec STM32
- **platformio_esp32_example** : Exemple pour PlatformIO avec ESP32

## Structure du projet

## Crédits et contributions

### Auteurs
- **Clément SAILLANT** - L'électron rare - *Développeur principal*
- **Szymon Łopaciuk** - *Travail initial et inspiration* - [stm32-adb2usb](https://github.com/szymonlopaciuk/stm32-adb2usb)

### Remerciements particuliers
- **Projet TMK** - Pour la documentation et le travail sur la conversion ADB-USB
- **Apple Developer Archives** - Pour la documentation technique sur le protocole ADB
- **Communauté r/VintageApple** - Pour le support et les tests

### Testeurs
- L'équipe de L'électron rare
- Utilisateurs du forum STM32duino
- Membres de la communauté RetroComputing

## Historique des versions

- **1.0.0-beta** (Janvier 2025)
  - Version initiale de la bibliothèque
  - Support complet pour STM32
  - Support préliminaire pour d'autres plateformes

## À venir

- Support complet pour Arduino, ESP32, et Teensy
- Meilleure gestion de l'alimentation
- Support pour des périphériques ADB plus exotiques
- Interface graphique de configuration et diagnostic

## Licence

Ce projet est sous licence GNU GPL v3. Voir le fichier LICENSE pour plus de détails.

---

Conçu avec ❤️ pour donner une nouvelle vie aux périphériques Apple vintage.

