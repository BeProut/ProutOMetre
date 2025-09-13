#ifndef AUDIO_RECORDER_H
#define AUDIO_RECORDER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <driver/i2s.h>
#include "../../config/audio_config.h"

// Configuration I2S
#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_BITS 16
#define I2S_READ_LEN 1024
#define I2S_CHANNEL_NUM 1
#define I2S_FORMAT I2S_CHANNEL_FMT_ONLY_LEFT

// Configuration bouton
#define DEBOUNCE_DELAY 50 // Délai anti-rebond en ms

class AudioRecorder
{
private:
    bool isRecording;
    bool buttonPressed;
    unsigned long buttonPressTime;
    unsigned long recordingStartTime;
    File audioFile;
    HTTPClient http;

    // Buffer pour les données audio
    int16_t *i2sBuffer;
    int16_t *processedBuffer;
    size_t bytesRead;

    // Variables pour gérer l'état du bouton
    bool lastButtonState;
    unsigned long lastDebounceTime;

    // Variables pour le traitement audio
    float currentGain;
    float targetGain;
    float maxSample;
    float avgPower;
    int sampleCount;

    // Filtre passe-haut simple (1er ordre)
    float highPassY1;
    float highPassX1;
    float highPassAlpha;

    void initI2S();
    void initLittleFS();
    void initButton();
    void initStatusLED();
    void setStatusLED(bool state);
    bool readButton();
    void startRecording();
    void stopRecording();
    void writeWAVHeader(File &file, uint32_t sampleRate, uint16_t bitsPerSample, uint16_t channels);
    void updateWAVHeader(File &file, uint32_t dataSize);
    bool uploadAudioFile();
    void processAudioData();

    // Nouvelles fonctions pour le traitement audio
    void initAudioProcessing();
    void applyHighPassFilter(int16_t *buffer, size_t samples);
    void applyAutoGainControl(int16_t *buffer, size_t samples);
    void normalizeAudio(int16_t *buffer, size_t samples);
    float calculateRMS(int16_t *buffer, size_t samples);

public:
    AudioRecorder();
    ~AudioRecorder();
    void begin();
    void process();
    bool isCurrentlyRecording() { return isRecording; }
};

#endif // AUDIO_RECORDER_H
