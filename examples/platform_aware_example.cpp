/**
 * @file platform_aware_example.cpp
 * @brief Exemple utilisant la détection automatique de plateforme
 * 
 * Cet exemple démontre l'utilisation du fichier ADBPlatform.h pour
 * créer un code qui s'adapte automatiquement à la plateforme cible.
 * 
 * @author Clément SAILLANT - L'électron rare
 * @copyright Copyright (C) 2025 Clément SAILLANT
 * @license GNU GPL v3
 */

#include <Arduino.h>
#include "adb.h"
#include "ADBPlatform.h"

// Utilisation des paramètres de la plateforme
ADB adb(ADB_DEFAULT_PIN);
ADBDevices devices(adb);
ADBUtils utils(devices);

void setup() {
  // Configuration adaptée à la plateforme
  Serial.begin(ADB_SERIAL_BAUD);
  while (!Serial && millis() < 3000);
  
  adb.init();
  
  // Affichage des informations de plateforme
  printPlatformInfo();
  Serial.println(F("Initialisation ADB réussie"));
  
  // Configuration spécifique selon la plateforme
  #ifdef ADB_PLATFORM_ESP32
    // Code spécifique pour ESP32
    Serial.println(F("Configuration spécifique pour ESP32 appliquée"));
  #elif defined(ADB_PLATFORM_STM32)
    // Code spécifique pour STM32
    Serial.println(F("Configuration spécifique pour STM32 appliquée"));
  #elif defined(ADB_PLATFORM_AVR)
    // Code spécifique pour Arduino AVR
    Serial.println(F("Mode économie de mémoire activé pour AVR"));
  #endif
  
  Serial.println(F("Prêt à recevoir les événements ADB"));
}

void loop() {
  // Lecture des périphériques
  utils.printKeyboardStatus();
  utils.printMouseStatus();
  
  // Utilisation de l'intervalle adapté à la plateforme
  delay(ADB_POLL_INTERVAL);
}
