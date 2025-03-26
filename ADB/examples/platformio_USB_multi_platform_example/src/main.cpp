#include <Arduino.h>
#include <ADB.h>
#include <ADBUtils.h>

#ifdef ARDUINO_ARCH_STM32
#include <USBHID.h> // STM32 USB HID
#elif defined(ARDUINO_ARCH_ESP32)
#include <USBHID.h> // ESP32 USB HID
#elif defined(ARDUINO_ARCH_AVR)
#include <HID-Project.h> // Arduino Leonardo/Micro HID
#endif

// Configuration des broches et paramètres
#ifdef ARDUINO_ARCH_STM32
constexpr uint8_t ADB_PIN = PB4; // STM32
#elif defined(ARDUINO_ARCH_ESP32)
constexpr uint8_t ADB_PIN = 21; // ESP32
#elif defined(ARDUINO_ARCH_AVR)
constexpr uint8_t ADB_PIN = 2; // Arduino Leonardo/Micro
#endif

constexpr uint16_t POLL_INTERVAL = 10; // 10ms (100Hz)

// Variables globales
ADB adb(ADB_PIN);
ADBDevices devices(adb);
ADBUtils utils(devices);

// États
bool keyboardConnected = false;
bool mouseConnected = false;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println(F("ADB2USB - Multi-Platform Example"));

  // Initialisation du bus ADB
  adb.init();

  // Initialisation USB HID
#ifdef ARDUINO_ARCH_STM32
  USBHID_begin(true, true);
#elif defined(ARDUINO_ARCH_ESP32)
  USBHID_begin(true, true);
#elif defined(ARDUINO_ARCH_AVR)
  Keyboard.begin();
  Mouse.begin();
#endif

  // Détection des périphériques
  bool error;
  keyboardConnected = !devices.keyboardReadModifiers(&error);
  mouseConnected = !devices.mouseReadData(&error);

  Serial.print(F("Clavier: "));
  Serial.println(keyboardConnected ? F("Connecté") : F("Déconnecté"));
  Serial.print(F("Souris: "));
  Serial.println(mouseConnected ? F("Connectée") : F("Déconnectée"));
}

void loop() {
  // Gestion du clavier
  if (keyboardConnected) {
    bool error = false;
    auto keyPress = devices.keyboardReadKeyPress(&error);
    if (error) {
      keyboardConnected = false;
      Serial.println(F("Clavier déconnecté"));
    } else {
#ifdef ARDUINO_ARCH_AVR
      // Gestion HID pour Arduino Leonardo/Micro
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
#else
      // Gestion HID pour STM32/ESP32
      uint8_t keyboardReport[8] = {0};
      keyboardReport[2] = ADBKeymap::toHID(keyPress.data.key0);
      USBHID_keyboard_report(keyboardReport);
#endif
    }
  }

  // Gestion de la souris
  if (mouseConnected) {
    bool error = false;
    auto mouseData = devices.mouseReadData(&error);
    if (error) {
      mouseConnected = false;
      Serial.println(F("Souris déconnectée"));
    } else {
      int8_t deltaX = adbMouseConvertAxis(mouseData.data.x_offset);
      int8_t deltaY = adbMouseConvertAxis(mouseData.data.y_offset);
      bool button = mouseData.data.button;

#ifdef ARDUINO_ARCH_AVR
      // Gestion HID pour Arduino Leonardo/Micro
      Mouse.move(deltaX, deltaY);
      if (button) {
        Mouse.press(MOUSE_LEFT);
      } else {
        Mouse.release(MOUSE_LEFT);
      }
#else
      // Gestion HID pour STM32/ESP32
      uint8_t mouseReport[4] = {0};
      mouseReport[0] = button ? 1 : 0;
      mouseReport[1] = deltaX;
      mouseReport[2] = deltaY;
      USBHID_mouse_report(mouseReport);
#endif
    }
  }

  delay(POLL_INTERVAL);
}
