/**
 * @file main.cpp
 * @brief Exemple PlatformIO pour STM32 avec la bibliothèque ADB
 * 
 * Cet exemple montre comment utiliser la bibliothèque ADB dans un projet PlatformIO
 * pour convertir un périphérique ADB en USB HID avec un STM32.
 * 
 * @author Clément SAILLANT - L'électron rare
 * @copyright Copyright (C) 2025 Clément SAILLANT
 * @license GNU GPL v3
 */

#include <Arduino.h>
#include <ADB.h>
#include <ADBUtils.h>
#include <USBHID.h>

// Configuration pour STM32
constexpr uint8_t ADB_PIN = PB4;        // Pin de données ADB
constexpr uint16_t POLL_INTERVAL = 10;  // 10ms (100Hz)

// Variables globales
ADB adb(ADB_PIN);
ADBDevices devices(adb);
ADBUtils utils(devices);

// États
bool keyboardConnected = false;
bool mouseConnected = false;
uint8_t keyboardReport[8] = {0};  // Modificateurs + touches
uint8_t mouseReport[4] = {0};     // Bouton, X, Y, Wheel

// Variables pour le suivi des mouvements de souris
int16_t accumulatedX = 0;
int16_t accumulatedY = 0;

void setup() {
  // Initialisation de la communication série
  Serial.begin(115200);
  delay(1000);
  
  Serial.println(F("ADB2USB pour STM32 - PlatformIO"));
  Serial.println(F("Bibliothèque ADB multiplateforme"));
  
  // Initialisation du bus ADB
  adb.init();
  
  // Initialisation USB HID (clavier + souris)
  USBHID_begin(true, true);
  
  // Détection des périphériques ADB
  bool error;
  
  // Test du clavier
  auto kbData = devices.keyboardReadModifiers(&error);
  keyboardConnected = !error;
  
  // Test de la souris
  error = false;
  auto mouseData = devices.mouseReadData(&error);
  mouseConnected = !error;
  
  Serial.print(F("Périphériques détectés - Clavier: "));
  Serial.print(keyboardConnected ? F("Oui") : F("Non"));
  Serial.print(F(", Souris: "));
  Serial.println(mouseConnected ? F("Oui") : F("Non"));
  
  Serial.println(F("Conversion ADB->USB active"));
}

void handleKeyboard() {
  if (!keyboardConnected) return;
  
  bool error = false;
  static uint8_t lastModifiers = 0;
  static uint8_t lastKeys[6] = {0};
  bool reportChanged = false;
  
  // Lecture des touches
  auto keyPress = devices.keyboardReadKeyPress(&error);
  if (error) {
    keyboardConnected = false;
    Serial.println(F("Clavier ADB déconnecté"));
    return;
  }
  
  // Lecture des modificateurs
  auto modifiers = devices.keyboardReadModifiers(&error);
  if (error) return;
  
  // Construction du rapport clavier USB HID
  uint8_t modByte = 0;
  if (modifiers.data.shift) modByte |= KEY_MOD_LSHIFT;
  if (modifiers.data.control) modByte |= KEY_MOD_LCTRL;
  if (modifiers.data.option) modByte |= KEY_MOD_LALT;
  if (modifiers.data.command) modByte |= KEY_MOD_LMETA;
  
  if (modByte != lastModifiers) {
    lastModifiers = modByte;
    reportChanged = true;
  }
  
  // Effacement des touches
  memset(&keyboardReport[2], 0, 6);
  
  // Ajout des touches
  uint8_t keyIdx = 0;
  
  if (keyPress.data.key0 && !keyPress.data.released0) {
    uint8_t hidCode = ADBKeymap::toHID(keyPress.data.key0);
    if (hidCode != KEY_NONE && !ADBKeymap::isModifier(keyPress.data.key0)) {
      keyboardReport[2 + keyIdx++] = hidCode;
    }
  }
  
  if (keyPress.data.key1 && !keyPress.data.released1) {
    uint8_t hidCode = ADBKeymap::toHID(keyPress.data.key1);
    if (hidCode != KEY_NONE && !ADBKeymap::isModifier(keyPress.data.key1)) {
      keyboardReport[2 + keyIdx++] = hidCode;
    }
  }
  
  // Vérification des changements
  for (uint8_t i = 0; i < 6; i++) {
    if (keyboardReport[2 + i] != lastKeys[i]) {
      lastKeys[i] = keyboardReport[2 + i];
      reportChanged = true;
    }
  }
  
  // Envoi du rapport si nécessaire
  if (reportChanged) {
    keyboardReport[0] = modByte;
    keyboardReport[1] = 0; // Reserved byte
    USBHID_keyboard_report(keyboardReport);
  }
}

void handleMouse() {
  if (!mouseConnected) return;
  
  bool error = false;
  static bool lastButton = false;
  
  // Lecture des données de la souris
  auto mouseData = devices.mouseReadData(&error);
  if (error) {
    mouseConnected = false;
    Serial.println(F("Souris ADB déconnectée"));
    return;
  }
  
  // Conversion des valeurs
  int8_t deltaX = adbMouseConvertAxis(mouseData.data.x_offset);
  int8_t deltaY = adbMouseConvertAxis(mouseData.data.y_offset);
  bool button = mouseData.data.button;
  
  // Accumulation des petits mouvements
  accumulatedX += deltaX;
  accumulatedY += deltaY;
  
  // Envoi du rapport souris si mouvement ou changement de bouton
  if (accumulatedX != 0 || accumulatedY != 0 || button != lastButton) {
    // Limitation des valeurs
    int8_t reportX = (accumulatedX > 127) ? 127 : ((accumulatedX < -127) ? -127 : accumulatedX);
    int8_t reportY = (accumulatedY > 127) ? 127 : ((accumulatedY < -127) ? -127 : accumulatedY);
    
    // Construction du rapport
    mouseReport[0] = button ? 1 : 0;  // Bouton gauche
    mouseReport[1] = reportX;
    mouseReport[2] = reportY;
    mouseReport[3] = 0;  // Molette
    
    // Envoi du rapport
    USBHID_mouse_report(mouseReport);
    
    // Mise à jour de l'état
    lastButton = button;
    
    // Réinitialisation des accumulateurs après envoi
    accumulatedX -= reportX;
    accumulatedY -= reportY;
  }
}

void updateLEDs() {
  // Synchronisation des LEDs USB avec le clavier ADB
  static uint8_t lastLEDs = 0;
  uint8_t currentLEDs = USBHID_get_status();
  
  if (currentLEDs != lastLEDs) {
    lastLEDs = currentLEDs;
    
    bool numLock = (currentLEDs & 0x01);
    bool capsLock = (currentLEDs & 0x02);
    bool scrollLock = (currentLEDs & 0x04);
    
    if (keyboardConnected) {
      devices.keyboardWriteLEDs(scrollLock, capsLock, numLock);
    }
  }
}

void reconnectDevices() {
  static uint32_t lastReconnectAttempt = 0;
  uint32_t now = millis();
  
  // Tentative de reconnexion toutes les 3 secondes
  if (now - lastReconnectAttempt > 3000) {
    lastReconnectAttempt = now;
    
    bool shouldReconnect = false;
    
    if (!keyboardConnected) {
      bool error = false;
      devices.keyboardReadModifiers(&error);
      if (!error) {
        keyboardConnected = true;
        Serial.println(F("Clavier ADB reconnecté"));
        shouldReconnect = true;
      }
    }
    
    if (!mouseConnected) {
      bool error = false;
      devices.mouseReadData(&error);
      if (!error) {
        mouseConnected = true;
        Serial.println(F("Souris ADB reconnectée"));
        shouldReconnect = true;
      }
    }
    
    if (shouldReconnect) {
      Serial.println(F("Périphériques ADB actifs:"));
      Serial.print(F("- Clavier: "));
      Serial.println(keyboardConnected ? F("Connecté") : F("Déconnecté"));
      Serial.print(F("- Souris: "));
      Serial.println(mouseConnected ? F("Connectée") : F("Déconnectée"));
    }
  }
}

void loop() {
  // Lecture et conversion des périphériques ADB
  handleKeyboard();
  handleMouse();
  updateLEDs();
  
  // Reconnexion si nécessaire
  if (!keyboardConnected || !mouseConnected) {
    reconnectDevices();
  }
  
  // Délai de polling optimal
  delay(POLL_INTERVAL);
}
