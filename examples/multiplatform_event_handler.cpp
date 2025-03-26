/**
 * @file multiplatform_event_handler.cpp
 * @brief Gestionnaire d'événements ADB multiplateforme
 * 
 * Cet exemple implémente un gestionnaire d'événements avancé pour les périphériques ADB.
 * Il est compatible avec toutes les plateformes prises en charge et montre comment traiter
 * les événements du clavier et de la souris de manière événementielle.
 * 
 * @author Clément SAILLANT - L'électron rare
 * @copyright Copyright (C) 2025 Clément SAILLANT
 * @license GNU GPL v3
 */

#include <Arduino.h>
#include "adb.h"

// Configuration spécifique à la plateforme
#if defined(ARDUINO_ARCH_AVR)
  #define ADB_PIN 2
#elif defined(ARDUINO_ARCH_STM32)
  #define ADB_PIN PB4
#elif defined(ARDUINO_ARCH_ESP32)
  #define ADB_PIN 21
#elif defined(TEENSYDUINO)
  #define ADB_PIN 3
#else
  #define ADB_PIN 2
#endif

// Paramètres communs
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint16_t POLL_INTERVAL = 20; // Intervalle court pour la réactivité

// Initialisation des objets
ADB adb(ADB_PIN);
ADBDevices devices(adb);

// État du système
uint8_t lastKeys[2] = {0, 0};
bool lastReleased[2] = {true, true};
int8_t lastX = 0, lastY = 0;
bool lastButton = false;

// Fonctions de traitement des événements
void onKeyPressed(uint8_t keyCode) {
  Serial.print(F("Touche pressée: 0x"));
  Serial.print(keyCode, HEX);
  
  // Affichage du code HID équivalent si disponible
  uint8_t hidCode = ADBKeymap::toHID(keyCode);
  if (hidCode != KEY_NONE) {
    Serial.print(F(" (HID: 0x"));
    Serial.print(hidCode, HEX);
    Serial.print(F(")"));
  }
  
  Serial.println();
}

void onKeyReleased(uint8_t keyCode) {
  Serial.print(F("Touche relâchée: 0x"));
  Serial.println(keyCode, HEX);
}

void onMouseMove(int8_t deltaX, int8_t deltaY) {
  Serial.print(F("Souris - Mouvement: ("));
  Serial.print(deltaX);
  Serial.print(F(", "));
  Serial.print(deltaY);
  Serial.println(F(")"));
}

void onMouseButtonChange(bool isPressed) {
  Serial.print(F("Souris - Bouton: "));
  Serial.println(isPressed ? F("Pressé") : F("Relâché"));
}

void setup() {
  // Configuration de la communication série
  Serial.begin(SERIAL_BAUD);
  while (!Serial && millis() < 3000);
  
  // Initialisation du bus ADB
  adb.init();
  Serial.println(F("Gestionnaire d'événements ADB multiplateforme initialisé"));
  Serial.println(F("En attente d'événements..."));
}

void loop() {
  // Traitement des événements clavier
  bool error = false;
  auto keyPress = devices.keyboardReadKeyPress(&error);
  
  if (!error) {
    // Vérification de la première touche
    if (keyPress.data.key0 != lastKeys[0] || keyPress.data.released0 != lastReleased[0]) {
      if (keyPress.data.key0 != 0) {
        if (!keyPress.data.released0 && lastReleased[0]) {
          onKeyPressed(keyPress.data.key0);
        } else if (keyPress.data.released0 && !lastReleased[0]) {
          onKeyReleased(keyPress.data.key0);
        }
      }
      lastKeys[0] = keyPress.data.key0;
      lastReleased[0] = keyPress.data.released0;
    }
    
    // Vérification de la seconde touche
    if (keyPress.data.key1 != lastKeys[1] || keyPress.data.released1 != lastReleased[1]) {
      if (keyPress.data.key1 != 0) {
        if (!keyPress.data.released1 && lastReleased[1]) {
          onKeyPressed(keyPress.data.key1);
        } else if (keyPress.data.released1 && !lastReleased[1]) {
          onKeyReleased(keyPress.data.key1);
        }
      }
      lastKeys[1] = keyPress.data.key1;
      lastReleased[1] = keyPress.data.released1;
    }
  }
  
  // Traitement des événements souris
  error = false;
  auto mouseData = devices.mouseReadData(&error);
  
  if (!error) {
    int8_t xMove = adbMouseConvertAxis(mouseData.data.x_offset);
    int8_t yMove = adbMouseConvertAxis(mouseData.data.y_offset);
    
    // Détection du mouvement
    if (xMove != 0 || yMove != 0) {
      onMouseMove(xMove, yMove);
    }
    
    // Détection du changement d'état du bouton
    if (mouseData.data.button != lastButton) {
      onMouseButtonChange(mouseData.data.button);
      lastButton = mouseData.data.button;
    }
  }
  
  // Fréquence de polling élevée pour une bonne réactivité
  delay(POLL_INTERVAL);
}
