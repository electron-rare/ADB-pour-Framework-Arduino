/**
 * @file ADBPlatform.h
 * @brief Détection automatique de plateforme pour la bibliothèque ADB
 * 
 * Ce fichier fournit des définitions et configurations automatiques basées 
 * sur la plateforme cible. Il peut être inclus dans des exemples pour une 
 * meilleure portabilité.
 * 
 * @author Clément SAILLANT - L'électron rare
 * @copyright Copyright (C) 2025 Clément SAILLANT
 * @license GNU GPL v3
 */

#ifndef ADB_PLATFORM_h
#define ADB_PLATFORM_h

// Détection de plateforme et configuration automatique
#if defined(ARDUINO_ARCH_AVR)
    #define ADB_PLATFORM_AVR
    #define ADB_PLATFORM_NAME "Arduino AVR"
    #define ADB_DEFAULT_PIN 2
    
#elif defined(ARDUINO_ARCH_SAM)
    #define ADB_PLATFORM_SAM
    #define ADB_PLATFORM_NAME "Arduino SAM"
    #define ADB_DEFAULT_PIN 2
    
#elif defined(ARDUINO_ARCH_SAMD)
    #define ADB_PLATFORM_SAMD
    #define ADB_PLATFORM_NAME "Arduino SAMD"
    #define ADB_DEFAULT_PIN 2
    
#elif defined(ARDUINO_ARCH_STM32)
    #define ADB_PLATFORM_STM32
    #define ADB_PLATFORM_NAME "STM32"
    // Sur les STM32, la broche par défaut est PB4
    #define ADB_DEFAULT_PIN PB4
    
#elif defined(ARDUINO_ARCH_ESP32)
    #define ADB_PLATFORM_ESP32
    #define ADB_PLATFORM_NAME "ESP32"
    #define ADB_DEFAULT_PIN 21
    
#elif defined(TEENSYDUINO)
    #define ADB_PLATFORM_TEENSY
    #define ADB_PLATFORM_NAME "Teensy"
    #define ADB_DEFAULT_PIN 3
    
#else
    #define ADB_PLATFORM_UNKNOWN
    #define ADB_PLATFORM_NAME "Plateforme inconnue"
    #define ADB_DEFAULT_PIN 2
    #warning "Plateforme non reconnue, utilisation de la pin 2 par défaut"
#endif

// Paramètres par défaut recommandés par plateforme
#ifdef ADB_PLATFORM_AVR
    // Arduino UNO, MEGA, etc. - MCU plus lent
    #define ADB_POLL_INTERVAL 100
    #define ADB_SERIAL_BAUD 9600
#elif defined(ADB_PLATFORM_STM32) || defined(ADB_PLATFORM_ESP32) || defined(ADB_PLATFORM_TEENSY)
    // Plateformes plus rapides
    #define ADB_POLL_INTERVAL 20
    #define ADB_SERIAL_BAUD 115200
#else
    // Valeurs par défaut pour autres plateformes
    #define ADB_POLL_INTERVAL 50
    #define ADB_SERIAL_BAUD 57600
#endif

/**
 * Fonction pour afficher les informations de plateforme
 */
inline void printPlatformInfo() {
    Serial.print(F("Bibliothèque ADB sur plateforme: "));
    Serial.println(F(ADB_PLATFORM_NAME));
    Serial.print(F("Pin ADB par défaut: "));
    Serial.println(ADB_DEFAULT_PIN);
}

#endif // ADB_PLATFORM_h
