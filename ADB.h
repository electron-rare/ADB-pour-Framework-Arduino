/**
 * @file ADB.h
 * @brief Bibliothèque multiplateforme pour la communication avec le bus Apple Desktop Bus (ADB)
 */

#ifndef ADB_MAIN_h
#define ADB_MAIN_h

#include <Arduino.h>
#include <cstdint>
#include "ADBKeymap.h"
#include "ADBKeyCodes.h"

namespace ADBProtocol {
    // Commandes ADB
    constexpr uint8_t CMD_TALK   = 0b11 << 2;
    constexpr uint8_t CMD_LISTEN = 0b10 << 2;
    constexpr uint8_t CMD_FLUSH  = 0b01 << 2;
    
    // Constantes diverses
    constexpr uint8_t BIT_ERROR  = 0xFF;
    constexpr uint8_t POLL_DELAY = 5;
    
    // Macros de conversion pour les adresses et registres ADB
    constexpr uint8_t ADDRESS(uint8_t addr) { return (addr << 4); }
    constexpr uint8_t REGISTER(uint8_t reg) { return reg; }
}

/**
 * @brief Structures de données pour les périphériques ADB
 */
struct adb_kb_keypress {
    uint8_t key1 : 7;      // Code de la deuxième touche
    bool released1 : 1;    // Indicateur de relâchement pour la deuxième touche
    uint8_t key0 : 7;      // Code de la première touche
    bool released0 : 1;    // Indicateur de relâchement pour la première touche
};

struct adb_kb_modifiers {
    bool led_num : 1;      // LED de verrouillage numérique
    bool led_caps : 1;     // LED de verrouillage majuscule
    bool led_scroll : 1;   // LED de défilement
    uint8_t reserved1 : 3; // Bits réservés
    bool scroll_lock : 1;  // État de verrouillage de défilement
    bool num_lock : 1;     // État de verrouillage numérique
    bool command : 1;      // Touche Command
    bool option : 1;       // Touche Option
    bool shift : 1;        // Touche Shift
    bool control : 1;      // Touche Control
    bool reset : 1;        // État de réinitialisation
    bool caps_lock : 1;    // État de verrouillage majuscule
    bool backspace : 1;    // Touche Backspace
    uint8_t reserved0 : 1; // Bit réservé
};

struct adb_mouse_data {
    uint8_t x_offset : 7;  // Déplacement horizontal de la souris
    uint8_t reserved0 : 1; // Bit réservé
    uint8_t y_offset : 7;  // Déplacement vertical de la souris
    bool button : 1;       // État du bouton de souris
};

struct adb_register3 {
    uint8_t device_handler_id : 8; // Identifiant du gestionnaire
    uint8_t device_address : 4;    // Adresse du périphérique
    uint8_t reserved1 : 1;         // Bit réservé
    uint8_t srq_enable : 1;        // Activation des requêtes de service
    uint8_t exceptional_event : 1; // Indicateur d'événement exceptionnel
    uint8_t reserved0 : 1;         // Bit réservé
};

/**
 * @brief Union pour faciliter l'accès aux données ADB sous forme brute ou structurée
 */
template <typename T>
union adb_data {
    uint16_t raw;
    T data;
};

/**
 * @brief Classe principale pour gérer le bus ADB
 */
class ADB {
public:
    /**
     * @brief Constructeur avec pin de données configurable
     * @param dataPin Broche utilisée pour les communications ADB
     */
    explicit ADB(uint8_t dataPin);
    
    /**
     * @brief Initialisation du bus ADB
     * @param dataPin Broche optionnelle pour reconfigurer la pin (0xFF pour conserver l'existante)
     * @param useADBDevices Indique si la classe ADBDevices sera utilisée
     */
    void init(uint8_t dataPin = 0xFF, bool useADBDevices = false);
    
    /**
     * @brief Réinitialise le bus ADB
     */
    void reset();
    
    /**
     * @brief Envoi d'une commande sur le bus ADB
     * @param command Code de commande ADB
     */
    void writeCommand(uint8_t command);
    
    /**
     * @brief Lecture de données depuis le bus ADB
     * @param buffer Pointeur vers le tampon de données
     * @param length Longueur des données à lire en bits
     * @return true si la lecture a réussi, false sinon
     */
    bool readDataPacket(uint16_t* buffer, uint8_t length);
    
    /**
     * @brief Écriture de données sur le bus ADB
     * @param bits Données à écrire
     * @param length Longueur des données à écrire en bits
     */
    void writeDataPacket(uint16_t bits, uint8_t length);
    
    /**
     * @brief Change la broche utilisée pour la communication
     * @param dataPin Nouvelle broche de données
     */
    void setPin(uint8_t dataPin);
    
    /**
     * @brief Attente de réponse du périphérique ADB
     * @param responseExpected Indique si une réponse est attendue
     * @return true si l'attente s'est bien déroulée
     */
    bool waitTLT(bool responseExpected);

private:
    uint8_t dataPin;        // Broche de données
    bool useADBDevices;     // Utilisation de la classe ADBDevices

    // Méthodes de bas niveau pour la communication ADB
    void wait();           // Signal d'attente (anciennement attention)
    void sync();            // Signal de synchronisation
    void writeBit(uint16_t bit); // Écriture d'un bit
    void writeBits(uint16_t bits, uint8_t length); // Écriture de plusieurs bits
    uint8_t readBit();      // Lecture d'un bit
};

/**
 * @brief Classe pour gérer les périphériques connectés au bus ADB
 */
class ADBDevices {
public:
    /**
     * @brief Constructeur avec référence à un objet ADB
     * @param adb Référence à l'instance ADB utilisée pour la communication
     */
    explicit ADBDevices(ADB& adb) : adb(adb) {}

    /**
     * @brief Initialisation d'un périphérique ADB
     * @param address Adresse du périphérique
     * @param handler_id Identifiant du gestionnaire
     * @param present Référence pour indiquer si le périphérique est présent
     * @return true si l'initialisation a réussi
     */
    bool initializeDevice(uint8_t address, uint8_t handler_id, bool& present);
    
    /**
     * @brief Lecture des modificateurs du clavier
     * @param error Pointeur pour indiquer si une erreur s'est produite
     * @return Structure contenant les modificateurs
     */
    adb_data<adb_kb_modifiers> keyboardReadModifiers(bool* error);
    
    /**
     * @brief Lecture des touches pressées sur le clavier
     * @param error Pointeur pour indiquer si une erreur s'est produite
     * @return Structure contenant les touches pressées
     */
    adb_data<adb_kb_keypress> keyboardReadKeyPress(bool* error);
    
    /**
     * @brief Configuration des LEDs du clavier
     * @param scroll État de la LED de défilement
     * @param caps État de la LED de verrouillage majuscule
     * @param num État de la LED de verrouillage numérique
     */
    void keyboardWriteLEDs(bool num, bool caps, bool scrool);
    
    /**
     * @brief Lecture des données de la souris
     * @param error Pointeur pour indiquer si une erreur s'est produite
     * @return Structure contenant les données de la souris
     */
    adb_data<adb_mouse_data> mouseReadData(bool* error);
    
    /**
     * @brief Mise à jour du registre 3 d'un périphérique
     * @param addr Adresse du périphérique
     * @param newReg3 Nouvelles valeurs pour le registre 3
     * @param mask Masque à appliquer
     * @param error Pointeur pour indiquer si une erreur s'est produite
     * @return true si la mise à jour a réussi
     */
    bool deviceUpdateRegister3(uint8_t addr, adb_data<adb_register3> newReg3, uint16_t mask, bool* error);

private:
    ADB& adb; // Référence à l'objet ADB utilisé pour la communication
    
    /**
     * @brief Lecture du registre 3 d'un périphérique
     * @param addr Adresse du périphérique
     * @param error Pointeur pour indiquer si une erreur s'est produite
     * @return Structure contenant les données du registre 3
     */
    adb_data<adb_register3> deviceReadRegister3(uint8_t addr, bool* error);
};

// Conversion des axes de la souris (format 7 bits en complément à 2 vers int8_t)
inline int8_t adbMouseConvertAxis(uint8_t value) {
    return static_cast<int8_t>(value << 1);
}

#endif // ADB_MAIN_h
