/**
 * @file usb_hid_stm32.cpp
 * @brief Exemple de conversion ADB vers USB HID pour STM32
 * 
 * Cet exemple montre comment convertir un clavier et une souris ADB en un périphérique
 * USB HID sur les plateformes STM32. Il permet d'utiliser des périphériques Apple vintage
 * sur des ordinateurs modernes via USB.
 *
 * @note Cet exemple nécessite un STM32 avec prise en charge USB (STM32F1, STM32F4, etc.)
 * 
 * @author Clément SAILLANT - L'électron rare
 * @copyright Copyright (C) 2025 Clément SAILLANT
 * @license GNU GPL v3
 */

#include <Arduino.h>
#include "adb.h"
#include "USBHID.h"  // Bibliothèque STM32 USB HID

// Configuration des broches
constexpr uint8_t ADB_PIN = PB4;        // Broche de données ADB

// Configuration des constantes
constexpr uint16_t POLL_INTERVAL = 10;  // Fréquence d'interrogation en ms (10ms = 100Hz)
constexpr uint8_t MAX_KEYS = 6;         // Nombre maximum de touches dans un rapport HID

// Initialisation des objets
ADB adb(ADB_PIN);
ADBDevices devices(adb);

// Buffers pour les rapports HID
uint8_t keyboardReport[8] = {0};  // Modificateurs (1) + réservé (1) + touches (6)
uint8_t mouseReport[4] = {0};     // Boutons (1) + X (1) + Y (1) + molette (1)

// État précédent des périphériques
uint8_t lastKeyboardKeys[MAX_KEYS] = {0};
uint8_t lastModifiers = 0;
int16_t mouseAccumulatedX = 0;
int16_t mouseAccumulatedY = 0;
bool lastButton = false;

// Indicateurs de présence des périphériques
bool keyboardPresent = false;
bool mousePresent = false;

/**
 * @brief Met à jour les LEDs du clavier ADB en fonction de l'état USB
 */
void updateKeyboardLEDs() {
    static uint8_t lastLEDState = 0;
    uint8_t currentLEDs = USBHID_get_status();
    
    if (currentLEDs != lastLEDState) {
        lastLEDState = currentLEDs;
        
        bool numLock = (currentLEDs & 0x01) != 0;
        bool capsLock = (currentLEDs & 0x02) != 0;
        bool scrollLock = (currentLEDs & 0x04) != 0;
        
        // Mise à jour des LEDs sur le clavier ADB
        if (keyboardPresent) {
            devices.keyboardWriteLEDs(scrollLock, capsLock, numLock);
        }
        
        Serial.print(F("LEDs mises à jour: "));
        Serial.print(capsLock ? F("CapsLock ") : F(""));
        Serial.print(numLock ? F("NumLock ") : F(""));
        Serial.println(scrollLock ? F("ScrollLock") : F(""));
    }
}

/**
 * @brief Détecte et initialise les périphériques ADB
 */
void detectADBDevices() {
    bool error = false;
    
    // Essai de lecture des modificateurs du clavier
    devices.keyboardReadModifiers(&error);
    keyboardPresent = !error;
    
    // Essai de lecture de la souris
    error = false;
    devices.mouseReadData(&error);
    mousePresent = !error;
    
    Serial.print(F("Détection ADB: Clavier: "));
    Serial.print(keyboardPresent ? F("Oui") : F("Non"));
    Serial.print(F(", Souris: "));
    Serial.println(mousePresent ? F("Oui") : F("Non"));
}

/**
 * @brief Lit les données du clavier ADB et les convertit en rapport USB HID
 */
void handleKeyboard() {
    if (!keyboardPresent) return;
    
    bool error = false;
    auto keyPress = devices.keyboardReadKeyPress(&error);
    
    if (error) {
        keyboardPresent = false;
        Serial.println(F("Erreur: Clavier ADB déconnecté"));
        return;
    }
    
    // Mise à jour des modificateurs (registre 2)
    auto modifiers = devices.keyboardReadModifiers(&error);
    if (error) return;
    
    // Calcul du nouvel état des modificateurs
    uint8_t newModifiers = 0;
    if (modifiers.data.shift) newModifiers |= KEY_MOD_LSHIFT;
    if (modifiers.data.control) newModifiers |= KEY_MOD_LCTRL;
    if (modifiers.data.option) newModifiers |= KEY_MOD_LALT;
    if (modifiers.data.command) newModifiers |= KEY_MOD_LMETA;
    
    // Construction du rapport clavier
    keyboardReport[0] = newModifiers;  // Octet des modificateurs
    
    // Effacer les touches
    memset(&keyboardReport[2], 0, MAX_KEYS);
    
    // Ajouter les touches pressées
    uint8_t keyIndex = 0;
    
    // Gestion de la touche 0
    if (keyPress.data.key0 != 0 && !keyPress.data.released0) {
        uint8_t hidCode = ADBKeymap::toHID(keyPress.data.key0);
        if (hidCode != KEY_NONE && !ADBKeymap::isModifier(keyPress.data.key0)) {
            keyboardReport[2 + keyIndex++] = hidCode;
        }
    }
    
    // Gestion de la touche 1
    if (keyPress.data.key1 != 0 && !keyPress.data.released1) {
        uint8_t hidCode = ADBKeymap::toHID(keyPress.data.key1);
        if (hidCode != KEY_NONE && !ADBKeymap::isModifier(keyPress.data.key1)) {
            keyboardReport[2 + keyIndex++] = hidCode;
        }
    }
    
    // Envoi du rapport uniquement s'il a changé
    bool changed = false;
    if (newModifiers != lastModifiers) {
        changed = true;
        lastModifiers = newModifiers;
    }
    
    for (uint8_t i = 0; i < MAX_KEYS; i++) {
        if (keyboardReport[2 + i] != lastKeyboardKeys[i]) {
            changed = true;
            lastKeyboardKeys[i] = keyboardReport[2 + i];
        }
    }
    
    if (changed) {
        USBHID_keyboard_report(keyboardReport);
    }
}

/**
 * @brief Lit les données de la souris ADB et les convertit en rapport USB HID
 */
void handleMouse() {
    if (!mousePresent) return;
    
    bool error = false;
    auto mouseData = devices.mouseReadData(&error);
    
    if (error) {
        mousePresent = false;
        Serial.println(F("Erreur: Souris ADB déconnectée"));
        return;
    }
    
    // Récupération des données souris
    int8_t deltaX = adbMouseConvertAxis(mouseData.data.x_offset);
    int8_t deltaY = adbMouseConvertAxis(mouseData.data.y_offset);
    bool buttonPressed = mouseData.data.button;
    
    // Cumul des petits mouvements pour éviter la perte de mouvements lents
    mouseAccumulatedX += deltaX;
    mouseAccumulatedY += deltaY;
    
    // Création du rapport souris seulement s'il y a un mouvement ou un changement d'état du bouton
    if (mouseAccumulatedX != 0 || mouseAccumulatedY != 0 || buttonPressed != lastButton) {
        mouseReport[0] = buttonPressed ? 1 : 0;  // Bouton gauche
        
        // Limitation des valeurs pour tenir dans un int8_t
        if (mouseAccumulatedX > 127) mouseAccumulatedX = 127;
        if (mouseAccumulatedX < -127) mouseAccumulatedX = -127;
        if (mouseAccumulatedY > 127) mouseAccumulatedY = 127;
        if (mouseAccumulatedY < -127) mouseAccumulatedY = -127;
        
        mouseReport[1] = (int8_t)mouseAccumulatedX;
        mouseReport[2] = (int8_t)mouseAccumulatedY;
        mouseReport[3] = 0;  // Pas de défilement
        
        // Envoi du rapport souris
        USBHID_mouse_report(mouseReport);
        
        // Réinitialisation des accumulateurs
        mouseAccumulatedX = 0;
        mouseAccumulatedY = 0;
        lastButton = buttonPressed;
    }
}

void setup() {
    // Initialisation de la communication série pour le débogage
    Serial.begin(115200);
    while (!Serial && millis() < 3000); // Attente avec timeout
    
    Serial.println(F("=== Convertisseur ADB vers USB HID pour STM32 ==="));
    Serial.println(F("Initialisation du bus ADB..."));
    
    // Initialisation du bus ADB
    adb.init();
    
    // Initialisation USB HID
    USBHID_begin(true, true);  // Activer clavier + souris
    
    // Détection des périphériques
    detectADBDevices();
    
    Serial.println(F("Initialisation terminée"));
    Serial.println(F("Le périphérique devrait maintenant être reconnu comme un clavier/souris USB"));
}

void loop() {
    // Lecture des périphériques ADB et envoi des rapports USB HID
    handleKeyboard();
    handleMouse();
    
    // Mise à jour des LEDs du clavier ADB en fonction de l'état du clavier USB
    updateKeyboardLEDs();
    
    // Si un périphérique a été déconnecté, tentative de reconnexion
    static uint32_t lastReconnectTime = 0;
    if ((!keyboardPresent || !mousePresent) && (millis() - lastReconnectTime > 1000)) {
        if (!keyboardPresent || !mousePresent) {
            Serial.println(F("Tentative de reconnexion des périphériques ADB..."));
            detectADBDevices();
            lastReconnectTime = millis();
        }
    }
    
    // Pause entre les lectures pour ne pas surcharger le bus
    delay(POLL_INTERVAL);
}
