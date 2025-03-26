#include <Arduino.h>
#include <ADB.h>
#include <ADBUtils.h>
#include <USBHID.h>

// Configuration pour Arduino Uno
constexpr uint8_t ADB_PIN = 2;          // Pin de données ADB
constexpr uint16_t POLL_INTERVAL = 10; // 10ms (100Hz)

// Variables globales
ADB adb(ADB_PIN);
ADBDevices devices(adb);
ADBUtils utils(devices);

// États
bool keyboardConnected = false;
bool mouseConnected = false;
uint8_t keyboardReport[8] = {0}; // Modificateurs + touches
uint8_t mouseReport[4] = {0};    // Bouton, X, Y, Wheel

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println(F("ADB2USB pour Arduino Uno - PlatformIO"));
  adb.init();
  USBHID_begin(true, true);

  bool error;
  keyboardConnected = !devices.keyboardReadModifiers(&error);
  mouseConnected = !devices.mouseReadData(&error);

  Serial.print(F("Clavier: "));
  Serial.println(keyboardConnected ? F("Connecté") : F("Déconnecté"));
  Serial.print(F("Souris: "));
  Serial.println(mouseConnected ? F("Connectée") : F("Déconnectée"));
}

void loop() {
  if (keyboardConnected) {
    bool error = false;
    auto keyPress = devices.keyboardReadKeyPress(&error);
    if (error) keyboardConnected = false;
    // ...traitement clavier...
  }

  if (mouseConnected) {
    bool error = false;
    auto mouseData = devices.mouseReadData(&error);
    if (error) mouseConnected = false;
    // ...traitement souris...
  }

  delay(POLL_INTERVAL);
}
