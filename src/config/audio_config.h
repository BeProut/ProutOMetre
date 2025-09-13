#ifndef AUDIO_CONFIG_H
#define AUDIO_CONFIG_H

// Configuration des pins I2S (ajustez selon votre microphone)
// Pour un microphone INMP441 typique :
#define I2S_SCK_IO 26 // Serial Clock (SCK/BCLK)
#define I2S_WS_IO 25  // Word Select (WS/LRCLK)
#define I2S_SD_IO 22  // Serial Data (SD/DOUT)

// Pour un microphone MAX9814 ou similaire :
// #define I2S_SCK_IO        14
// #define I2S_WS_IO         15
// #define I2S_SD_IO         32

// Configuration du bouton
#define BUTTON_PIN 21 // Pin du bouton (utilisez un bouton avec pull-up)

// Configuration LED de statut (optionnel)
#define STATUS_LED_PIN 2 // Pin de la LED de statut (GPIO2 = LED intégrée sur la plupart des ESP32)
#define LED_ENABLED true // Mettre à false pour désactiver la LED

// Configuration audio
#define I2S_SAMPLE_RATE 16000 // Fréquence d'échantillonnage (Hz) - Retour à 16kHz pour éviter pitch shift
#define MAX_RECORD_TIME 30000 // Temps max d'enregistrement (ms)

// Configuration avancée pour améliorer la qualité audio
#define I2S_DMA_BUF_COUNT 16 // Nombre de buffers DMA (plus = moins de dropouts)
#define I2S_DMA_BUF_LEN 256  // Taille de chaque buffer DMA
#define AUDIO_GAIN 1.0       // Gain appliqué à l'audio (1.0 = pas de gain)
#define HIGH_PASS_CUTOFF 50  // Fréquence de coupure du filtre passe-haut (Hz) - Abaissée pour plus de naturel

// Configuration pour la normalisation automatique
#define AUTO_GAIN_ENABLED false // Désactivé pour tests - peut causer des artefacts
#define TARGET_VOLUME 0.7      // Volume cible pour la normalisation (0.0 à 1.0)
#define AGC_ATTACK_TIME 0.1    // Temps d'attaque du AGC en secondes
#define AGC_RELEASE_TIME 0.5   // Temps de relâchement du AGC en secondes

// Configuration serveur
#define SERVER_URL "http://192.168.0.18:3000/audio/upload"

// Configuration LittleFS
#define AUDIO_FILE_PATH "/recording.wav"

#endif // AUDIO_CONFIG_H
