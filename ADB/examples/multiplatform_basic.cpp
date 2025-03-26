/**
 * @file multiplatform_basic.cpp
 * @brief Exemple de base pour l'utilisation de la bibliothèque ADB sur plusieurs plateformes
 * 
 * Cet exemple montre comment utiliser la bibliothèque ADB sur différentes plateformes
 * en modifiant simplement la définition de la plateforme et la broche utilisée.
 * 
 * @author Clément SAILLANT - L'électron rare
 * @copyright Copyright (C) 2025 Clément SAILLANT
 * @license GNU GPL v3
 */

#include <Arduino.h>
#include "adb.h"

// Définir la plateforme (décommenter une seule ligne)
#define PLATFORM_ARDUINO
// #define PLATFORM_STM32
// #define PLATFORM_ESP32
// #define PLATFORM_TEENSY

// Configuration de la broche ADB selon la plateforme
#ifdef PLATFORM_ARDUINO
  #define ADB_PIN 2
  #define PLATFORM_NAME "Arduino"
#elif defined(PLATFORM_STM32)
  #define ADB_PIN PB4
  #define PLATFORM_NAME "STM32"
#elif defined(PLATFORM_ESP32)
  #define ADB_PIN 21
  #define PLATFORM_NAME "ESP32"
#elif defined(PLATFORM_TEENSY)
  #define ADB_PIN 3
  #define PLATFORM_NAME "Teensy"
#else
  #error "Aucune plateforme définie! Décommentez une définition de plateforme."
#endif

// Paramètres communs
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint16_t POLL_INTERVAL = 250; // ms

// Initialisation des objets
ADB adb(ADB_PIN);
ADBDevices devices(adb);

void setup() {
  // Configuration de la communication série
  Serial.begin(SERIAL_BAUD);
  while (!Serial && millis() < 3000); // Attente du port série avec timeout
  
  // Initialisation du bus ADB
  adb.init();
  
  Serial.print(F("ADB Initialized on "));
  Serial.println(F(PLATFORM_NAME));
  Serial.println(F("Bibliothèque ADB multiplateforme"));
}

void loop() {
  bool error = false;
  
  // Tentative de lecture du clavier
  auto keyPress = devices.keyboardReadKeyPress(&error);
  if (!error && (keyPress.data.key0 != 0)) {
    Serial.print(F("Keyboard - Key: 0x"));
    Serial.print(keyPress.data.key0, HEX);
    Serial.print(F(" ("));
    Serial.print(keyPress.data.released0 ? F("Released") : F("Pressed"));
    Serial.println(F(")"));
  }
  
  // Tentative de lecture de la souris
  error = false;
  auto mouseData = devices.mouseReadData(&error);
  if (!error) {
    int8_t xMove = adbMouseConvertAxis(mouseData.data.x_offset);
    int8_t yMove = adbMouseConvertAxis(mouseData.data.y_offset);
    
    if (xMove != 0 || yMove != 0 || mouseData.data.button) {
      Serial.print(F("Mouse - X: "));
      Serial.print(xMove);
      Serial.print(F(", Y: "));
      Serial.print(yMove);
      Serial.print(F(", Button: "));
      Serial.println(mouseData.data.button ? F("Pressed") : F("Released"));
    }
  }
  
  delay(POLL_INTERVAL);
}
