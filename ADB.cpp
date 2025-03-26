/**
 * @file ADB.cpp
 * @brief Implémentation de la bibliothèque ADB multiplateforme
 *
 * Cette implémentation offre les fonctionnalités nécessaires pour communiquer avec
 * des périphériques Apple Desktop Bus sur différentes plateformes (Arduino, STM32, ESP32, Teensy).
 * Le code est optimisé pour fonctionner de manière cohérente indépendamment du matériel sous-jacent.
 *
 * @author Clément SAILLANT - L'électron rare
 * @copyright Copyright (C) 2025 Clément SAILLANT
 * @license GNU GPL v3
 */

#include "ADB.h"

/**
 * Implémentation de la classe ADB - Gestion du bus Apple Desktop Bus
 */

ADB::ADB(uint8_t dataPin) : dataPin(dataPin), useADBDevices(false) {}

void ADB::init(uint8_t dataPin, bool useADBDevices) {
    // Mise à jour de la broche si spécifiée
    if (dataPin != 0xFF) {
        this->dataPin = dataPin;
    }
    this->useADBDevices = useADBDevices;

    // Configuration de la broche en mode open-drain
    pinMode(this->dataPin, OUTPUT_OPEN_DRAIN);
    digitalWrite(this->dataPin, HIGH);

    // Attente que la ligne soit prête
    while (digitalRead(this->dataPin) == LOW) {
        // Attendre que la ligne remonte
    }

    // Réinitialisation du bus
    reset();
}

void ADB::reset() {
    // Signal de réinitialisation: maintenir la ligne basse pendant 3ms
    digitalWrite(dataPin, LOW);
    delayMicroseconds(3000);
    digitalWrite(dataPin, HIGH);
}

void ADB::wait() {
    // Signal d'attente: maintenir la ligne basse pendant 800µs
    digitalWrite(dataPin, LOW);
    delayMicroseconds(800);
    digitalWrite(dataPin, HIGH);
}

void ADB::sync() {
    // Signal de synchronisation pour les commandes
    digitalWrite(dataPin, HIGH);
    delayMicroseconds(70);
    digitalWrite(dataPin, LOW);
}

void ADB::writeBit(uint16_t bit) {
    // Encodage Manchester modifié:
    // 1 = 35µs bas puis 65µs haut
    // 0 = 65µs bas puis 35µs haut
    if (bit) {
        digitalWrite(dataPin, LOW);
        delayMicroseconds(35);
        digitalWrite(dataPin, HIGH);
        delayMicroseconds(65);
    } else {
        digitalWrite(dataPin, LOW);
        delayMicroseconds(65);
        digitalWrite(dataPin, HIGH);
        delayMicroseconds(35);
    }
}

void ADB::writeBits(uint16_t bits, uint8_t length) {
    // Écrit plusieurs bits, du MSB au LSB
    uint16_t mask = 1 << (length - 1);
    while (mask) {
        writeBit(bits & mask);
        mask >>= 1;
    }
}

void ADB::writeDataPacket(uint16_t bits, uint8_t length) {
    // Format du paquet: bit de début (1), données, bit de fin (0)
    writeBit(1);
    writeBits(bits, length);
    writeBit(0);
}

bool ADB::waitTLT(bool responseExpected) {
    // Attend la réponse d'un périphérique après une commande
    digitalWrite(dataPin, HIGH);
    delayMicroseconds(140);
    
    // Si une réponse est attendue, attendre jusqu'à 240µs
    if (responseExpected) {
        uint8_t timeout = 0;
        while (digitalRead(dataPin) == HIGH && timeout < 240) {
            delayMicroseconds(1);
            timeout++;
        }
    }
    return true;
}

uint8_t ADB::readBit() {
    // Lecture d'un bit avec détection de timeout
    const unsigned long MAX_WAIT = 85; // Microseconds
    
    // Attente du front montant
    auto time_start = micros();
    while (digitalRead(dataPin) == LOW) {
        if (micros() - time_start > MAX_WAIT)
            return ADBProtocol::BIT_ERROR;
    }
    auto low_time = micros() - time_start;

    // Attente du front descendant
    time_start = micros();
    while (digitalRead(dataPin) == HIGH) {
        if (micros() - time_start > MAX_WAIT)
            return ADBProtocol::BIT_ERROR;
    }
    auto high_time = micros() - time_start;

    // Décodage Manchester modifié
    return (low_time < high_time) ? 0x1 : 0x0;
}

bool ADB::readDataPacket(uint16_t* buffer, uint8_t length) {
    // Vérifie le bit de début
    if (readBit() != 0x1) {
        return false;
    }

    // Lecture bit par bit des données
    *buffer = 0;
    for (uint8_t i = 0; i < length; i++) {
        uint8_t current_bit = readBit();
        if (current_bit == ADBProtocol::BIT_ERROR) {
            return false;
        }
        *buffer = (*buffer << 1) | current_bit;
    }

    // Lecture du bit de fin (ignoré)
    readBit();
    return true;
}

void ADB::writeCommand(uint8_t command) {
    wait();
    sync();
    writeBits(static_cast<uint16_t>(command), 8);
    writeBit(0);
}

void ADB::setPin(uint8_t dataPin) {
    this->dataPin = dataPin;
    pinMode(dataPin, OUTPUT_OPEN_DRAIN);
    digitalWrite(dataPin, HIGH);
}

/**
 * Implémentation de la classe ADBDevices - Gestion des périphériques ADB
 */

bool ADBDevices::initializeDevice(uint8_t address, uint8_t handler_id, bool& present) {
    bool error = false;
    
    // Préparer les données pour le registre 3
    adb_data<adb_register3> reg3 = {0};
    adb_data<adb_register3> mask = {0};
    
    reg3.data.device_handler_id = handler_id;
    mask.data.device_handler_id = 0xFF;

    // Tenter de configurer le périphérique et vérifier sa présence
    present = deviceUpdateRegister3(address, reg3, mask.raw, &error) && !error;
    return present;
}

adb_data<adb_kb_modifiers> ADBDevices::keyboardReadModifiers(bool* error) {
    adb_data<adb_kb_modifiers> modifiers = {0};
    
    // Envoi d'une commande Talk au registre 2 du clavier
    adb.writeCommand(ADBProtocol::CMD_TALK | ADBProtocol::ADDRESS(ADBKey::Address::KEYBOARD) | ADBProtocol::REGISTER(2));
    adb.waitTLT(true);
    
    // Lecture des données et mise à jour du statut d'erreur
    *error = !adb.readDataPacket(&modifiers.raw, 16);
    return modifiers;
}

adb_data<adb_kb_keypress> ADBDevices::keyboardReadKeyPress(bool* error) {
    adb_data<adb_kb_keypress> keyPress = {0};
    
    // Envoi d'une commande Talk au registre 0 du clavier
    adb.writeCommand(ADBProtocol::CMD_TALK | ADBProtocol::ADDRESS(ADBKey::Address::KEYBOARD) | ADBProtocol::REGISTER(0));
    adb.waitTLT(true);
    
    // Lecture des touches pressées et mise à jour du statut d'erreur
    *error = !adb.readDataPacket(&keyPress.raw, 16);
    return keyPress;
}

void ADBDevices::keyboardWriteLEDs(bool scroll, bool caps, bool num) {
    adb_data<adb_kb_modifiers> modifiers = {0};
    
    // Configuration des LEDs (la logique est inversée dans le protocole)
    modifiers.data.led_scroll = !scroll;
    modifiers.data.led_caps = !caps;
    modifiers.data.led_num = !num;

    // Envoi d'une commande Listen au registre 2 du clavier
    adb.writeCommand(ADBProtocol::CMD_LISTEN | ADBProtocol::ADDRESS(ADBKey::Address::KEYBOARD) | ADBProtocol::REGISTER(2));
    adb.waitTLT(false);
    
    // Envoi des données de configuration des LEDs
    adb.writeDataPacket(modifiers.raw, 16);
}

adb_data<adb_mouse_data> ADBDevices::mouseReadData(bool* error) {
    adb_data<adb_mouse_data> mouseData = {0};
    
    // Envoi d'une commande Talk au registre 0 de la souris
    adb.writeCommand(ADBProtocol::CMD_TALK | ADBProtocol::ADDRESS(ADBKey::Address::MOUSE) | ADBProtocol::REGISTER(0));
    adb.waitTLT(true);
    
    // Lecture des données de la souris et mise à jour du statut d'erreur
    *error = !adb.readDataPacket(&mouseData.raw, 16);
    return mouseData;
}

adb_data<adb_register3> ADBDevices::deviceReadRegister3(uint8_t addr, bool* error) {
    adb_data<adb_register3> reg3 = {0};
    
    // Envoi d'une commande Talk au registre 3 du périphérique
    adb.writeCommand(ADBProtocol::CMD_TALK | ADBProtocol::ADDRESS(ADBKey::Address::KEYBOARD) | ADBProtocol::REGISTER(3));
    adb.waitTLT(true);
    
    // Lecture de la configuration du périphérique
    *error = !adb.readDataPacket(&reg3.raw, 16);
    return reg3;
}

bool ADBDevices::deviceUpdateRegister3(uint8_t addr, adb_data<adb_register3> newReg3, uint16_t mask, bool* error) {
    // Lecture de la configuration actuelle
    adb_data<adb_register3> reg3 = deviceReadRegister3(addr, error);
    if (*error) return false;
    
    // Attente entre les opérations
    delay(POLL_DELAY);

    // Application du masque pour ne modifier que les bits souhaités
    reg3.raw = (reg3.raw & ~mask) | (newReg3.raw & mask);

    // Envoi d'une commande Listen pour mettre à jour la configuration
    adb.writeCommand(ADBProtocol::CMD_LISTEN | ADBProtocol::ADDRESS(ADBKey::Address::KEYBOARD) | ADBProtocol::REGISTER(3));
    adb.waitTLT(false);
    adb.writeDataPacket(reg3.raw, 16);
    
    // Attente entre les opérations
    delay(POLL_DELAY);

    // Vérification que la mise à jour a été prise en compte
    reg3 = deviceReadRegister3(addr, error);
    if (*error) return false;

    return (reg3.raw & mask) == (newReg3.raw & mask);
}
