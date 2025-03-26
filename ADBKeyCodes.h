/**
 * Définition des codes de touches ADB
 * Adaptées du protocole Apple Desktop Bus
 */

#ifndef ADB_KEYCODES_H
#define ADB_KEYCODES_H

#include <cstdint>

/**
 * Constantes du protocole Apple Desktop Bus
 */
namespace ADBKey {
    namespace Address {
        constexpr uint8_t KEYBOARD = 0x2;
        constexpr uint8_t MOUSE = 0x3;
    }
    
    namespace KeyCode {
        // Touches spéciales
        constexpr uint8_t CAPS_LOCK = 0x39;
        constexpr uint8_t NUM_LOCK = 0x47;
        
        // Touches de modification
        constexpr uint8_t LEFT_SHIFT = 0x38;
        constexpr uint8_t RIGHT_SHIFT = 0x7B;
        constexpr uint8_t LEFT_CONTROL = 0x36;
        constexpr uint8_t RIGHT_CONTROL = 0x7D;
        constexpr uint8_t LEFT_OPTION = 0x3A;
        constexpr uint8_t RIGHT_OPTION = 0x7C;
        constexpr uint8_t LEFT_COMMAND = 0x37;
        constexpr uint8_t RIGHT_COMMAND = 0x37;
        
        // Touche Power
        constexpr uint8_t POWER_KEY_CODE = 0x66;
        constexpr uint16_t POWER_DOWN = 0x7F7F;
        constexpr uint16_t POWER_UP = 0xFFFF;
    }
}

#endif // ADB_KEYCODES_H
