/**
 * @file ADBCore.h
 * @brief Fichier principal d'inclusion pour la bibliothèque ADB multiplateforme
 */

#ifndef ADB_CORE_h
#define ADB_CORE_h

// Ordre d'inclusion optimisé pour assurer que toutes les dépendances sont disponibles
#include "HIDTables.h"      // Définitions des codes HID
#include "ADBKeyCodes.h"    // Définitions des constantes ADB
#include "ADBKeymap.h"      // Mappage ADB vers HID
#include "ADB.h"            // Interface principale du protocole ADB
#include "ADBUtils.h"       // Utilitaires supplémentaires

// Macro pour convertir les valeurs des axes souris (pour la compatibilité avec le code existant)
#define ADB_MOUSE_CONV_AXIS(val) adbMouseConvertAxis(val)

#endif // ADB_CORE_h
