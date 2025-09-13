#ifndef AUDIO_CONFIG_TEST_H
#define AUDIO_CONFIG_TEST_H

// Configuration des pins I2S (ajustez selon votre microphone)
// Pour un microphone INMP441 typique :
#define I2S_SCK_IO 26 // Serial Clock (SCK/BCLK)
#define I2S_WS_IO 25  // Word Select (WS/LRCLK)
#define I2S_SD_IO 22  // Serial Data (SD/DOUT)

// Configuration du bouton
#define BUTTON_PIN 21 // Pin du bouton (utilisez un bouton avec pull-up)

// Configuration LED de statut (optionnel)
#define STATUS_LED_PIN 2 // Pin de la LED de statut
#define LED_ENABLED true // Mettre à false pour désactiver la LED

// CONFIGURATION AUDIO POUR TESTS - SON NATUREL
#define I2S_SAMPLE_RATE 16000   // Fréquence standard
#define MAX_RECORD_TIME 30000   // Temps max d'enregistrement (ms)

// Configuration basique pour son naturel
#define I2S_DMA_BUF_COUNT 8     // Valeurs standards
#define I2S_DMA_BUF_LEN 512     // Valeurs standards
#define AUDIO_GAIN 1.0          // Pas de gain
#define HIGH_PASS_CUTOFF 20     // Très bas pour garder le naturel

// TOUS LES TRAITEMENTS DÉSACTIVÉS POUR TESTS
#define AUTO_GAIN_ENABLED false // Désactivé
#define TARGET_VOLUME 0.7       
#define AGC_ATTACK_TIME 0.1     
#define AGC_RELEASE_TIME 0.5    

// Configuration serveur
#define SERVER_URL "http://192.168.0.18:3000/audio/upload"

// Configuration LittleFS
#define AUDIO_FILE_PATH "/recording.wav"

#endif // AUDIO_CONFIG_TEST_H
