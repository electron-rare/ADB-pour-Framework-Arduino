/**
 * @file ADBKeymap.h
 * @brief Constantes et fonctions de conversion pour la bibliothèque ADB multiplateforme
 */

#ifndef ADB_KEYMAP_h
#define ADB_KEYMAP_h

#include <cstdint>
#include "HIDTables.h"
#include "ADBKeyCodes.h"

// Classe pour la conversion ADB vers HID
class ADBKeymap {
public:
    // Vérifie si une touche est un modificateur
    static bool isModifier(uint8_t key) {
        using namespace ADBKey::KeyCode;
        return 
            (key == LEFT_SHIFT) ||
            (key == RIGHT_SHIFT) ||
            (key == LEFT_OPTION) ||
            (key == RIGHT_OPTION) ||
            (key == LEFT_COMMAND) ||
            (key == RIGHT_COMMAND) ||
            (key == LEFT_CONTROL) ||
            (key == RIGHT_CONTROL);
    }

    // Obtient le masque du modificateur correspondant à une touche ADB
    static uint8_t getModifierMask(uint8_t adbKeycode) {
        using namespace ADBKey::KeyCode;
        if (adbKeycode == LEFT_SHIFT) return KEY_MOD_LSHIFT;
        if (adbKeycode == RIGHT_SHIFT) return KEY_MOD_RSHIFT;
        if (adbKeycode == LEFT_CONTROL) return KEY_MOD_LCTRL;
        if (adbKeycode == RIGHT_CONTROL) return KEY_MOD_RCTRL;
        if (adbKeycode == LEFT_OPTION) return KEY_MOD_LALT;
        if (adbKeycode == RIGHT_OPTION) return KEY_MOD_RALT;
        if (adbKeycode == LEFT_COMMAND) return KEY_MOD_LMETA;
        if (adbKeycode == RIGHT_COMMAND) return KEY_MOD_RMETA;
        return 0;
    }

    // Convertit un code ADB en code HID
    static uint8_t toHID(uint8_t adbKeycode) {
        if (adbKeycode >= 128) return KEY_NONE;
        return keyCodeTable[adbKeycode];
    }
    
    // Tableau de conversion ADB vers HID
    static const uint8_t keyCodeTable[128];
};

#endif // ADB_KEYMAP_h
