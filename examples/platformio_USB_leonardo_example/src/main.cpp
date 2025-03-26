#include <Arduino.h>
#include <ADB.h>
#include <ADBUtils.h>
#include <HID-Project.h> // Bibliothèque HID pour Arduino Leonardo/Micro

// Configuration pour Arduino Leonardo/Micro
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

  Serial.println(F("ADB2USB pour Arduino Leonardo/Micro - PlatformIO"));
  adb.init();

  // Initialisation HID
  Keyboard.begin();
  Mouse.begin();

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
    if (error) {
      keyboardConnected = false;
      Serial.println(F("Clavier déconnecté"));
    } else {
      // Traitement des touches
      if (keyPress.data.key0 && !keyPress.data.released0) {
        uint8_t hidCode = ADBKeymap::toHID(keyPress.data.key0);
        if (hidCode != KEY_NONE) {
          Keyboard.press(hidCode);
        }
      }
      if (keyPress.data.released0) {
        uint8_t hidCode = ADBKeymap::toHID(keyPress.data.key0);
        if (hidCode != KEY_NONE) {
          Keyboard.release(hidCode);
        }
      }
    }
  }

  if (mouseConnected) {
    bool error = false;
    auto mouseData = devices.mouseReadData(&error);
    if (error) {
      mouseConnected = false;
      Serial.println(F("Souris déconnectée"));
    } else {
      // Traitement des mouvements de souris
      int8_t deltaX = adbMouseConvertAxis(mouseData.data.x_offset);
      int8_t deltaY = adbMouseConvertAxis(mouseData.data.y_offset);
      bool button = mouseData.data.button;

      Mouse.move(deltaX, deltaY);
      if (button) {
        Mouse.press(MOUSE_LEFT);
      } else {
        Mouse.release(MOUSE_LEFT);
      }
    }
  }

  delay(POLL_INTERVAL);
}
