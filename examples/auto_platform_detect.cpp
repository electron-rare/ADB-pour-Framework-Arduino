/**
 * @file auto_platform_detect.cpp
 * @brief Exemple avec détection automatique de la plateforme matérielle
 * 
 * Ce fichier démontre comment la bibliothèque ADB peut être utilisée avec
 * une détection automatique de la plateforme matérielle sous-jacente.
 * 
 * @author Clément SAILLANT - L'électron rare
 * @copyright Copyright (C) 2025 Clément SAILLANT
 * @license GNU GPL v3
 */

#include <Arduino.h>
#include "adb.h"

// Détection automatique de plateforme et configuration de la broche
#if defined(ARDUINO_ARCH_AVR)
  #define PLATFORM_NAME "Arduino AVR"
  #define ADB_PIN 2
#elif defined(ARDUINO_ARCH_SAM)
  #define PLATFORM_NAME "Arduino SAM"
  #define ADB_PIN 2
#elif defined(ARDUINO_ARCH_SAMD)
  #define PLATFORM_NAME "Arduino SAMD"
  #define ADB_PIN 2
#elif defined(ARDUINO_ARCH_STM32)
  #define PLATFORM_NAME "STM32"
  #define ADB_PIN PB4
#elif defined(ARDUINO_ARCH_ESP32)
  #define PLATFORM_NAME "ESP32"
  #define ADB_PIN 21
#elif defined(TEENSYDUINO)
  #define PLATFORM_NAME "Teensy"
  #define ADB_PIN 3
#else
  #define PLATFORM_NAME "Unknown Platform"
  #define ADB_PIN 2
  #warning "Plateforme non reconnue, utilisation de la pin 2 par défaut"
#endif

// Paramètres communs
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint16_t POLL_INTERVAL = 250; // ms

// Initialisation des objets
ADB adb(ADB_PIN);
ADBDevices devices(adb);
ADBUtils utils(devices);

void setup() {
  // Configuration de la communication série
  Serial.begin(SERIAL_BAUD);
  while (!Serial && millis() < 3000); // Attente avec timeout
  
  // Initialisation du bus ADB
  adb.init();
  
  Serial.println(F("=== ADB Multiplateforme ==="));
  Serial.print(F("Plateforme détectée: "));
  Serial.println(F(PLATFORM_NAME));
  Serial.print(F("Utilisation de la broche: "));
  Serial.println(ADB_PIN);
  Serial.println(F("========================"));
}

void loop() {
  // Affichage des informations périphériques ADB
  utils.printKeyboardStatus();
  utils.printMouseStatus();
  
  Serial.println(F("------------------------"));
  delay(POLL_INTERVAL);
}
