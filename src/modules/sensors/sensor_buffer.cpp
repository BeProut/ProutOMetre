#include "sensor_buffer.h"

// ======== INSTANCE GLOBALE ========
static SensorBuffer sensorBuffer;

// ======== IMPLÉMENTATION DE LA CLASSE SensorBuffer ========

void SensorBuffer::addSensorData(const SensorData &sensorData)
{
    // Convertir les valeurs brutes en unités physiques
    addSensorData(
        mq135ToPPM(sensorData.mq135Value),
        mq136ToPPM(sensorData.mq136Value),
        mq4ToPPM(sensorData.mq4Value),
        max4466ToDecibels(sensorData.max4466Value));
}

void SensorBuffer::addSensorData(float mq135, float mq136, float mq4, float mic)
{
    SensorRecord record;
    record.timestamp = millis();
    record.mq135_ppm = mq135;
    record.mq136_ppm = mq136;
    record.mq4_ppm = mq4;
    record.mic_db = mic;

    if (size < MAX_BUFFER_SIZE)
    {
        // Buffer pas encore plein
        buffer[(startIndex + size) % MAX_BUFFER_SIZE] = record;
        size++;
    }
    else
    {
        // Buffer plein -> écrase le plus ancien élément
        buffer[startIndex] = record;
        startIndex = (startIndex + 1) % MAX_BUFFER_SIZE;
    }

    lastSampleTime = millis();
}

void SensorBuffer::addToJson(JsonDocument &doc)
{
    JsonArray sensorsArray = doc.createNestedArray("sensors");

    for (int i = 0; i < size; i++)
    {
        int idx = (startIndex + i) % MAX_BUFFER_SIZE;
        JsonObject sensorJson = sensorsArray.createNestedObject();
        sensorJson["timestamp"] = buffer[idx].timestamp;
        sensorJson["mq135_ppm"] = buffer[idx].mq135_ppm;
        sensorJson["mq136_ppm"] = buffer[idx].mq136_ppm;
        sensorJson["mq4_ppm"] = buffer[idx].mq4_ppm;
        sensorJson["mic_db"] = buffer[idx].mic_db;
    }
}

bool SensorBuffer::shouldSample()
{
    return (millis() - lastSampleTime) >= SENSOR_SAMPLING_INTERVAL;
}

void SensorBuffer::sampleIfNeeded()
{
    if (shouldSample())
    {
        SensorData currentData = getAllSensorData();
        addSensorData(currentData);
    }
}

void SensorBuffer::clear()
{
    size = 0;
    startIndex = 0;
}

SensorRecord SensorBuffer::getAverage() const
{
    if (size == 0)
    {
        return {0, 0.0, 0.0, 0.0, 0.0};
    }

    SensorRecord avg = {0, 0.0, 0.0, 0.0, 0.0};

    for (int i = 0; i < size; i++)
    {
        int idx = (startIndex + i) % MAX_BUFFER_SIZE;
        avg.mq135_ppm += buffer[idx].mq135_ppm;
        avg.mq136_ppm += buffer[idx].mq136_ppm;
        avg.mq4_ppm += buffer[idx].mq4_ppm;
        avg.mic_db += buffer[idx].mic_db;
    }

    avg.mq135_ppm /= size;
    avg.mq136_ppm /= size;
    avg.mq4_ppm /= size;
    avg.mic_db /= size;
    avg.timestamp = millis();

    return avg;
}

// ======== FONCTIONS D'INITIALISATION ========

void sensorBufferInit()
{
    Serial.println("Sensor Buffer: Initialisation du buffer des capteurs...");
    sensorBuffer.clear();
    Serial.printf("Sensor Buffer: Buffer initialisé avec une taille maximale de %d éléments\n", MAX_BUFFER_SIZE);
    Serial.printf("Sensor Buffer: Intervalle d'échantillonnage: %d ms\n", SENSOR_SAMPLING_INTERVAL);
}

void sensorBufferProcess()
{
    sensorBuffer.sampleIfNeeded();
}

// ======== FONCTIONS D'ACCÈS GLOBALES ========

void addCurrentSensorDataToBuffer()
{
    SensorData currentData = getAllSensorData();
    sensorBuffer.addSensorData(currentData);
}

void getSensorBufferJson(JsonDocument &doc)
{
    sensorBuffer.addToJson(doc);
}

void clearSensorBuffer()
{
    Serial.printf("Sensor Buffer: Vidage du buffer (%d éléments supprimés)\n", sensorBuffer.getSize());
    sensorBuffer.clear();
}

int getSensorBufferSize()
{
    return sensorBuffer.getSize();
}