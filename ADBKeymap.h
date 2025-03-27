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
        if (adbKeycode == LEFT_SHIFT) return ADB_KEY_MOD_LSHIFT;
        if (adbKeycode == RIGHT_SHIFT) return ADB_KEY_MOD_RSHIFT;
        if (adbKeycode == LEFT_CONTROL) return ADB_KEY_MOD_LCTRL;
        if (adbKeycode == RIGHT_CONTROL) return ADB_KEY_MOD_RCTRL;
        if (adbKeycode == LEFT_OPTION) return ADB_KEY_MOD_LALT;
        if (adbKeycode == RIGHT_OPTION) return ADB_KEY_MOD_RALT;
        if (adbKeycode == LEFT_COMMAND) return ADB_KEY_MOD_LMETA;
        if (adbKeycode == RIGHT_COMMAND) return ADB_KEY_MOD_RMETA;
        return 0;
    }

    // Convertit un code ADB en code HID
    static uint8_t toHID(uint8_t adbKeycode) {
        if (adbKeycode >= 128) return ADB_KEY_NONE;
        return keyCodeTable[adbKeycode];
    }
    
    /**
     * @brief Vérifie si une touche appartient au pavé numérique.
     * @param hid_keycode Code HID de la touche.
     * @return true si la touche appartient au pavé numérique, false sinon.
     */
    static bool isNumericKeypadKey(uint8_t hid_keycode);

    /**
     * @brief Vérifie si une touche est une touche fonction (F1 à F15).
     * @param hid_keycode Code HID de la touche.
     * @return true si la touche est une touche fonction, false sinon.
     */
    static bool isFunctionKey(uint8_t hid_keycode);

    // Tableau de conversion ADB vers HID
    static const uint8_t keyCodeTable[128];

};

#endif // ADB_KEYMAP_h
