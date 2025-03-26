/**
 * @file multiplatform_device_info.cpp
 * @brief Exemple de récupération d'informations sur les périphériques ADB
 * 
 * Ce programme permet de détecter et d'afficher des informations sur les périphériques
 * ADB connectés. Il est compatible avec toutes les plateformes supportées.
 * 
 * @author Clément SAILLANT - L'électron rare
 * @copyright Copyright (C) 2025 Clément SAILLANT
 * @license GNU GPL v3
 */

#include <Arduino.h>
#include "adb.h"

// Configuration multiplateforme
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM)
  #define ADB_PIN 2
  #define PLATFORM_NAME "Arduino"
#elif defined(ARDUINO_ARCH_STM32)
  #define ADB_PIN PB4
  #define PLATFORM_NAME "STM32"
#elif defined(ARDUINO_ARCH_ESP32)
  #define ADB_PIN 21
  #define PLATFORM_NAME "ESP32"
#elif defined(TEENSYDUINO)
  #define ADB_PIN 3
  #define PLATFORM_NAME "Teensy"
#else
  #define ADB_PIN 2
  #define PLATFORM_NAME "Plateforme inconnue"
#endif

// Adresses ADB à tester
constexpr uint8_t DEVICE_ADDRESSES[] = {2, 3, 4, 5, 6, 7};
constexpr uint8_t NUM_ADDRESSES = sizeof(DEVICE_ADDRESSES) / sizeof(DEVICE_ADDRESSES[0]);

// Initialisation des objets
ADB adb(ADB_PIN);
ADBDevices devices(adb);

void setup() {
  // Configuration de la communication série
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
  
  Serial.println(F("=== Scanner de périphériques ADB ==="));
  Serial.print(F("Plateforme: "));
  Serial.print(F(PLATFORM_NAME));
  Serial.print(F(", Broche ADB: "));
  Serial.println(ADB_PIN);
  
  // Initialisation du bus ADB
  adb.init();
  delay(500); // Attendre que le bus se stabilise
  
  Serial.println(F("Recherche de périphériques ADB..."));
  Serial.println(F("--------------------------------"));
  
  // Tentative de détection de périphériques
  bool deviceFound = false;
  
  for (uint8_t i = 0; i < NUM_ADDRESSES; i++) {
    uint8_t addr = DEVICE_ADDRESSES[i];
    bool error = false;
    bool devicePresent = false;
    
    Serial.print(F("Adresse 0x"));
    Serial.print(addr, HEX);
    Serial.print(F(" : "));
    
    // Essai de lecture du registre 3
    adb_data<adb_register3> reg3 = {0};
    adb.writeCommand(ADBProtocol::CMD_TALK | ADBProtocol::ADDRESS(addr) | ADBProtocol::REGISTER(3));
    adb.waitTLT(true);
    bool success = adb.readDataPacket(&reg3.raw, 16);
    
    if (success) {
      Serial.println(F("Périphérique détecté!"));
      Serial.print(F("  Handler ID: 0x"));
      Serial.println(reg3.data.device_handler_id, HEX);
      
      // Identification du type de périphérique
      if (addr == ADDR_KEYBOARD) {
        Serial.println(F("  Type: Clavier ADB"));
        // Tentative de lecture des modificateurs
        bool keyError = false;
        auto modifiers = devices.keyboardReadModifiers(&keyError);
        if (!keyError) {
          Serial.println(F("  État Caps Lock: ") + 
                         String(modifiers.data.caps_lock ? "Activé" : "Désactivé"));
        }
      } 
      else if (addr == ADDR_MOUSE) {
        Serial.println(F("  Type: Souris ADB"));
      }
      else {
        Serial.print(F("  Type: Autre périphérique ADB ("));
        Serial.print(addr);
        Serial.println(F(")"));
      }
      
      deviceFound = true;
    } else {
      Serial.println(F("Aucun périphérique"));
    }
  }
  
  if (!deviceFound) {
    Serial.println(F("Aucun périphérique ADB détecté."));
    Serial.println(F("Vérifiez les connexions et l'alimentation."));
  }
  
  Serial.println(F("--------------------------------"));
  Serial.println(F("Scan terminé. Surveillance active..."));
}

void loop() {
  // Surveillance continue des périphériques
  bool keyboardError = false;
  bool mouseError = false;
  static bool keyboardPresent = false;
  static bool mousePresent = false;
  
  // Vérification du clavier
  auto keyPress = devices.keyboardReadKeyPress(&keyboardError);
  if (!keyboardError) {
    if (!keyboardPresent) {
      Serial.println(F("Clavier ADB connecté"));
      keyboardPresent = true;
    }
    
    // Affichage des touches pressées
    if (keyPress.data.key0 != 0 && !keyPress.data.released0) {
      Serial.print(F("Touche: 0x"));
      Serial.println(keyPress.data.key0, HEX);
    }
  } else if (keyboardPresent) {
    Serial.println(F("Clavier ADB déconnecté"));
    keyboardPresent = false;
  }
  
  // Vérification de la souris
  auto mouseData = devices.mouseReadData(&mouseError);
  if (!mouseError) {
    if (!mousePresent) {
      Serial.println(F("Souris ADB connectée"));
      mousePresent = true;
    }
    
    // Affichage des mouvements significatifs
    int8_t xMove = adbMouseConvertAxis(mouseData.data.x_offset);
    int8_t yMove = adbMouseConvertAxis(mouseData.data.y_offset);
    
    if (abs(xMove) > 5 || abs(yMove) > 5) {
      Serial.print(F("Mouvement: X="));
      Serial.print(xMove);
      Serial.print(F(", Y="));
      Serial.println(yMove);
    }
  } else if (mousePresent) {
    Serial.println(F("Souris ADB déconnectée"));
    mousePresent = false;
  }
  
  delay(100);
}
