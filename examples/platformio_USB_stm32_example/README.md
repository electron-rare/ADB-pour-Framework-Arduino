# ADB2USB pour STM32 - Convertisseur Apple Desktop Bus vers USB

![Version](https://img.shields.io/badge/version-1.0-blue.svg)
![Licence](https://img.shields.io/badge/licence-GPL%20v3-green.svg)
![Plateforme](https://img.shields.io/badge/plateforme-STM32-red.svg)
![Framework](https://img.shields.io/badge/framework-Arduino-brightgreen.svg)

<p align="center">
  <img src="https://raw.githubusercontent.com/electron-rare/stm32-adb2usb/main/docs/assets/adb2usb-logo.png" alt="Logo ADB2USB" width="300" />
</p>

> Donnez une seconde vie à vos périphériques Apple vintage en les connectant à des ordinateurs modernes !

## 📋 Table des matières

- [À propos du projet](#à-propos)
- [Fonctionnalités](#fonctionnalités)
- [Matériel requis](#matériel-requis)
- [Schéma de branchement](#branchements)
- [Installation](#installation)
- [Utilisation](#utilisation)
- [Personnalisation](#personnalisation)
- [Débogage](#débogage)
- [Contribution](#contribution)
- [Licence](#licence)
- [Contact](#contact)

## ℹ️ À propos

Cette implémentation permet de connecter des anciens claviers et souris Apple utilisant le bus ADB à un ordinateur moderne via USB. Le microcontrôleur STM32 agit comme un pont de conversion, interprétant les signaux ADB et les transformant en signaux USB HID.

## ✨ Fonctionnalités

- ⌨️ Prise en charge des claviers ADB (touches et modificateurs)
- 🖱️ Prise en charge des souris ADB (déplacements et boutons)
- 🔍 Détection automatique des périphériques ADB
- 🔄 Reconnexion automatique des périphériques déconnectés
- 💡 Synchronisation des LED (Verr. Maj, etc.) entre USB et ADB
- 📦 Exemple complet pour PlatformIO

## 🔌 Matériel requis

- Carte de développement STM32 compatible avec Arduino (par exemple, STM32 Blue Pill)
- Périphériques ADB (clavier et/ou souris Apple)
- Connecteur ADB (Mini-DIN 4 broches)
- Résistance pull-up de 4.7kΩ à 10kΩ pour la ligne de données ADB
- Câble USB pour connecter la carte STM32 à l'ordinateur

## 🔧 Branchements

```
ADB Mini-DIN 4 broches:
  - Pin 1 (ADB Data) → STM32 Pin PB4 (avec résistance pull-up 4.7kΩ à 10kΩ vers 5V)
  - Pin 2 (Power SW) → Non connecté
  - Pin 3 (5V) → 5V
  - Pin 4 (GND) → GND
```

<details>
  <summary>Schéma de câblage</summary>
  <img src="https://raw.githubusercontent.com/electron-rare/stm32-adb2usb/main/docs/assets/wiring.png" alt="Schéma de câblage" width="600" />
</details>

## 🚀 Installation

### Prérequis logiciels

- [PlatformIO](https://platformio.org/) (installé comme extension VS Code ou en ligne de commande)
- Environnement de développement STM32 pour Arduino

### Étapes d'installation

1. Cloner ce dépôt
   ```bash
   git clone https://github.com/electron-rare/stm32-adb2usb.git
   cd stm32-adb2usb
   ```

2. Ouvrir le dossier dans PlatformIO
   ```bash
   code .
   ```

3. Compiler et téléverser vers votre carte
   ```bash
   pio run -t upload
   ```

## 📝 Utilisation

Une fois le code téléversé:

1. Connectez votre périphérique ADB à la carte STM32
2. Connectez la carte STM32 à votre ordinateur via USB
3. Les périphériques ADB seront automatiquement détectés et fonctionneront comme des périphériques USB HID

## ⚙️ Personnalisation

Pour adapter ce code à votre matériel spécifique:
- Modifiez la constante `ADB_PIN` pour correspondre à votre brochage
- Ajustez `POLL_INTERVAL` selon vos besoins de réactivité

## 🐛 Débogage

Vous pouvez suivre le processus de détection et la connexion des périphériques via le moniteur série:

```bash
pio device monitor -b 115200
```

## 👥 Contribution

Les contributions sont les bienvenues ! N'hésitez pas à ouvrir une issue ou une pull request.

1. Forkez le projet
2. Créez votre branche (`git checkout -b feature/amazing-feature`)
3. Commitez vos changements (`git commit -m 'Add some amazing feature'`)
4. Pushez vers la branche (`git push origin feature/amazing-feature`)
5. Ouvrez une Pull Request

## 📄 Licence

Ce projet est sous licence GNU GPL v3. Voir le fichier `LICENSE` pour plus de détails.

## 📧 Contact

Clément SAILLANT - [@lelectron_rare](https://twitter.com/lelectron_rare) - contact@lelectron-rare.fr

Lien du projet: [https://github.com/electron-rare/stm32-adb2usb](https://github.com/electron-rare/stm32-adb2usb)
