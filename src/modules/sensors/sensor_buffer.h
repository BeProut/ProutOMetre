#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <modules/sensors/sensors_manager.h>

// ======== CONFIGURATION ========
#define MAX_BUFFER_SIZE 100
#define SENSOR_SAMPLING_INTERVAL 100 // Intervalle d'échantillonnage en ms

// ======== STRUCTURE ========
struct SensorRecord
{
    uint32_t timestamp;
    float mq135_ppm;
    float mq136_ppm;
    float mq4_ppm;
    float mic_db;
};

// ======== BUFFER ========
class SensorBuffer
{
private:
    SensorRecord buffer[MAX_BUFFER_SIZE];
    int size = 0;                // nombre d'éléments actuels
    int startIndex = 0;          // index du plus ancien élément
    uint32_t lastSampleTime = 0; // dernière fois qu'un échantillon a été pris

public:
    // Ajouter une nouvelle lecture avec conversion automatique
    void addSensorData(const SensorData &sensorData);

    // Ajouter une nouvelle lecture avec valeurs déjà converties
    void addSensorData(float mq135, float mq136, float mq4, float mic);

    // Ajouter les données au JSON
    void addToJson(JsonDocument &doc);

    // Vérifier si il faut prendre un nouvel échantillon
    bool shouldSample();

    // Prendre un échantillon automatiquement si nécessaire
    void sampleIfNeeded();

    // Obtenir le nombre d'éléments dans le buffer
    int getSize() const { return size; }

    // Vider le buffer après envoi
    void clear();

    // Obtenir la moyenne des dernières valeurs (pour debug)
    SensorRecord getAverage() const;
};

// ======== FONCTIONS D'INITIALISATION ========
void sensorBufferInit();
void sensorBufferProcess();

// ======== FONCTIONS D'ACCÈS ========
void addCurrentSensorDataToBuffer();
void getSensorBufferJson(JsonDocument &doc);
void clearSensorBuffer();
int getSensorBufferSize();