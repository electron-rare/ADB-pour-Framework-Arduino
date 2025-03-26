/**
 * @file main.cpp
 * @brief Exemple PlatformIO pour ESP32 avec la bibliothèque ADB
 * 
 * Cet exemple montre comment utiliser la bibliothèque ADB dans un projet PlatformIO
 * avec l'ESP32 pour lire et traiter les données des périphériques ADB.
 * 
 * @author Clément SAILLANT - L'électron rare
 * @copyright Copyright (C) 2025 Clément SAILLANT
 * @license GNU GPL v3
 */

#include <Arduino.h>
#include <ADB.h>
#include <ADBUtils.h>
#include <BLEDevice.h>
#include <BLEHIDDevice.h>
#include <HIDTypes.h>

// Configuration pour ESP32
constexpr uint8_t ADB_PIN = 21;        // Pin de données ADB
constexpr uint16_t POLL_INTERVAL = 20;  // 20ms (50Hz)
constexpr const char* DEVICE_NAME = "ADB2BLE Adapter";

// Variables globales
ADB adb(ADB_PIN);
ADBDevices devices(adb);
ADBUtils utils(devices);

// BLE HID
BLEHIDDevice* hid;
BLECharacteristic* inputKeyboard;
BLECharacteristic* inputMouse;
bool connected = false;

// États
bool keyboardConnected = false;
bool mouseConnected = false;
uint8_t keyboardReport[8] = {0};  // Modificateurs + touches
uint8_t mouseReport[4] = {0};     // Bouton, X, Y, Wheel

// Variables pour le suivi des mouvements de souris
int16_t accumulatedX = 0;
int16_t accumulatedY = 0;

// Callback pour la connexion BLE
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* server) {
    connected = true;
    Serial.println(F("Client BLE connecté"));
  }
  
  void onDisconnect(BLEServer* server) {
    connected = false;
    Serial.println(F("Client BLE déconnecté"));
    BLEDevice::startAdvertising();
  }
};

void setupBLE() {
  // Initialisation du BLE
  BLEDevice::init(DEVICE_NAME);
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());
  
  // Création du service HID
  hid = new BLEHIDDevice(server);
  inputKeyboard = hid->inputReport(1); // Report ID 1: Keyboard
  inputMouse = hid->inputReport(2);    // Report ID 2: Mouse
  
  // Configuration des descripteurs HID
  hid->manufacturer()->setValue("L'électron rare");
  hid->pnp(0x02, 0x05AC, 0x8240, 0x0001); // Apple
  hid->hidInfo(0x00, 0x01);
  
  // Définition des descripteurs de rapport
  const uint8_t reportMapKeyboard[] = {
    // Clavier
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x05, 0x07,        //   Usage Page (Key Codes)
    0x19, 0xE0,        //   Usage Minimum (224)
    0x29, 0xE7,        //   Usage Maximum (231)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x01,        //   Input (Constant)
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x65,        //   Logical Maximum (101)
    0x05, 0x07,        //   Usage Page (Key Codes)
    0x19, 0x00,        //   Usage Minimum (0)
    0x29, 0x65,        //   Usage Maximum (101)
    0x81, 0x00,        //   Input (Data, Array)
    0xC0,              // End Collection
    
    // Souris
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x02,        //   Report ID (2)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (1)
    0x29, 0x03,        //     Usage Maximum (3)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x03,        //     Report Count (3)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data, Variable, Absolute)
    0x95, 0x01,        //     Report Count (1)
    0x75, 0x05,        //     Report Size (5)
    0x81, 0x01,        //     Input (Constant)
    0x05, 0x01,        //     Usage Page (Generic Desktop)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x09, 0x38,        //     Usage (Wheel)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x03,        //     Report Count (3)
    0x81, 0x06,        //     Input (Data, Variable, Relative)
    0xC0,              //   End Collection
    0xC0               // End Collection
  };
  
  hid->reportMap((uint8_t*)reportMapKeyboard, sizeof(reportMapKeyboard));
  
  // Activation du service
  hid->startServices();
  
  // Démarrage de la publicité BLE
  BLEAdvertising* advertising = server->getAdvertising();
  advertising->setAppearance(HID_KEYBOARD);
  advertising->addServiceUUID(hid->hidService()->getUUID());
  advertising->start();
  
  Serial.println(F("BLE HID prêt, en attente de connexion"));
}

void setup() {
  // Initialisation de la communication série
  Serial.begin(115200);
  delay(1000);
  
  Serial.println(F("ADB2BLE pour ESP32 - PlatformIO"));
  Serial.println(F("Bibliothèque ADB multiplateforme"));
  
  // Initialisation du bus ADB
  adb.init();
  
  // Configuration BLE
  setupBLE();
  
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
  
  Serial.println(F("Conversion ADB->BLE active"));
}

void handleKeyboard() {
  if (!keyboardConnected || !connected) return;
  
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
  
  // Construction du rapport clavier BLE HID
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
    inputKeyboard->setValue(keyboardReport, 8);
    inputKeyboard->notify();
  }
}

void handleMouse() {
  if (!mouseConnected || !connected) return;
  
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
    inputMouse->setValue(mouseReport, 4);
    inputMouse->notify();
    
    // Mise à jour de l'état
    lastButton = button;
    
    // Réinitialisation des accumulateurs après envoi
    accumulatedX -= reportX;
    accumulatedY -= reportY;
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
  
  // Reconnexion si nécessaire
  if (!keyboardConnected || !mouseConnected) {
    reconnectDevices();
  }
  
  // Délai de polling optimal
  delay(POLL_INTERVAL);
}
