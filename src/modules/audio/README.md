# Module d'Enregistrement Audio - ProutOMètre

## Description
Ce module permet d'enregistrer de l'audio via un microphone I2S et de l'envoyer automatiquement à un serveur HTTP lorsqu'un bouton est relâché.

## Fonctionnalités
- ✅ Enregistrement audio via I2S (16kHz, 16-bit, mono)
- ✅ Gestion d'un bouton avec anti-rebond
- ✅ Stockage temporaire sur LittleFS (formatage automatique au démarrage)
- ✅ Upload automatique vers serveur HTTP
- ✅ Format WAV standard
- ✅ Limitation du temps d'enregistrement (30s par défaut)

## Câblage

### Microphone I2S (ex: INMP441)
```
ESP32 Pin    | Microphone Pin | Description
-------------|----------------|-------------
GPIO 26      | SCK/BCLK       | Serial Clock
GPIO 25      | WS/LRCLK       | Word Select  
GPIO 33      | SD/DOUT        | Serial Data
3.3V         | VDD            | Alimentation
GND          | GND            | Masse
GND          | L/R            | Sélection canal (GND = Left)
```

### Bouton
```
ESP32 Pin    | Bouton         | Description
-------------|----------------|-------------
GPIO 4       | Pin 1          | Signal (avec pull-up interne)
GND          | Pin 2          | Masse
```

## Configuration
Modifiez le fichier `src/config/audio_config.h` pour adapter :
- Les pins I2S selon votre microphone
- Le pin du bouton
- L'URL du serveur
- Les paramètres audio

## Utilisation
1. **Appui sur le bouton** : Démarre l'enregistrement
2. **Relâchement du bouton** : Arrête l'enregistrement et envoie au serveur
3. **LED de statut** : (optionnel) Ajouter une LED pour indiquer l'état

## API Serveur Attendue
Le serveur doit accepter une requête POST sur `/audio/upload` avec :
- Content-Type: `audio/wav`
- Body: Fichier WAV complet

## Dépendances
- ESP32 Arduino Core
- LittleFS (inclus)
- HTTPClient (inclus)
- Driver I2S (inclus)

## Troubleshooting

### Pas de son enregistré
- Vérifiez le câblage I2S
- Testez avec un autre microphone
- Vérifiez l'alimentation du microphone

### Bouton ne répond pas
- Vérifiez le câblage du bouton
- Ajustez `DEBOUNCE_DELAY` si nécessaire
- Testez avec un multimètre

### Erreur d'upload
- Vérifiez que le WiFi est connecté
- Testez l'URL du serveur manuellement
- Vérifiez les logs série pour plus d'infos

### Problème LittleFS
- Le système formate automatiquement au démarrage
- Vérifiez l'espace disponible
- Redémarrez l'ESP32 si nécessaire

## Exemple de logs
```
[AUDIO] Initialisation du module audio...
[AUDIO] LittleFS monté avec succès
[AUDIO] Bouton initialisé sur le pin 4
[AUDIO] I2S initialisé avec succès
[AUDIO] Module audio initialisé avec succès
[AUDIO] Début de l'enregistrement...
[AUDIO] Enregistrement en cours...
[AUDIO] Arrêt de l'enregistrement...
[AUDIO] Fichier audio sauvegardé (54432 octets)
[AUDIO] Envoi du fichier audio au serveur...
[AUDIO] Fichier envoyé avec succès au serveur
```
