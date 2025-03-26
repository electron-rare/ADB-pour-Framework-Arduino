/**
 * @file ADBUtils.h
 * @brief Utilitaires pour la bibliothèque ADB multiplateforme
 *
 * Ce fichier fournit des classes et fonctions utilitaires pour simplifier
 * l'utilisation de la bibliothèque ADB sur différentes plateformes matérielles.
 * Les utilitaires sont conçus pour être compatibles avec toutes les plateformes supportées.
 *
 * @author Clément SAILLANT - L'électron rare
 * @copyright Copyright (C) 2025 Clément SAILLANT
 * @license GNU GPL v3
 */

#ifndef ADB_UTILS_h
#define ADB_UTILS_h

#include <Arduino.h>
#include "ADB.h"

/**
 * @brief Classe d'utilitaires pour faciliter l'utilisation des périphériques ADB
 */
class ADBUtils {
public:
    /**
     * @brief Constructeur avec référence à un objet ADBDevices
     * @param devices Référence à l'instance ADBDevices
     */
    explicit ADBUtils(ADBDevices& devices) : devices(devices) {}
    
    /**
     * @brief Imprime l'état des touches du clavier sur le port série
     * @return true si la lecture a réussi
     */
    bool printKeyboardStatus() {
        bool error = false;
        
        // Lecture des modificateurs
        auto modifiers = devices.keyboardReadModifiers(&error);
        if (error) return false;
        
        Serial.println(F("Keyboard Status:"));
        Serial.print(F("  Caps Lock: "));
        Serial.println(modifiers.data.led_caps ? F("ON") : F("OFF"));
        Serial.print(F("  Num Lock: "));
        Serial.println(modifiers.data.led_num ? F("ON") : F("OFF"));
        
        // Lecture des touches pressées
        auto keyPress = devices.keyboardReadKeyPress(&error);
        if (error) return false;
        
        if (keyPress.data.key0 != 0) {
            Serial.print(F("  Key 0: 0x"));
            Serial.print(keyPress.data.key0, HEX);
            Serial.print(F(" ("));
            Serial.print(keyPress.data.released0 ? F("Released") : F("Pressed"));
            Serial.println(F(")"));
        }
        
        if (keyPress.data.key1 != 0) {
            Serial.print(F("  Key 1: 0x"));
            Serial.print(keyPress.data.key1, HEX);
            Serial.print(F(" ("));
            Serial.print(keyPress.data.released1 ? F("Released") : F("Pressed"));
            Serial.println(F(")"));
        }
        
        return true;
    }
    
    /**
     * @brief Imprime les données de la souris sur le port série
     * @return true si la lecture a réussi
     */
    bool printMouseStatus() {
        bool error = false;
        
        // Lecture des données de la souris
        auto mouseData = devices.mouseReadData(&error);
        if (error) return false;
        
        // Conversion des valeurs brutes
        int8_t xMove = adbMouseConvertAxis(mouseData.data.x_offset);
        int8_t yMove = adbMouseConvertAxis(mouseData.data.y_offset);
        
        Serial.println(F("Mouse Status:"));
        Serial.print(F("  X Movement: "));
        Serial.println(xMove);
        Serial.print(F("  Y Movement: "));
        Serial.println(yMove);
        Serial.print(F("  Button: "));
        Serial.println(mouseData.data.button ? F("Pressed") : F("Released"));
        
        return true;
    }
    
private:
    ADBDevices& devices;
};

#endif // ADB_UTILS_h
