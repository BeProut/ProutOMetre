#include "sensors_manager.h"
#include "../../config/pins_config.h"

// Variables pour le filtrage et la stabilisation des lectures
static unsigned long lastReadTime = 0;
static const unsigned long READ_INTERVAL = 100; // Lecture toutes les 100ms

// Variables pour moyenner les lectures (filtre simple)
static uint16_t mq135Buffer[5] = {0};
static uint16_t mq136Buffer[5] = {0};
static uint16_t mq4Buffer[5] = {0};
static uint16_t max4466Buffer[5] = {0};
static uint8_t bufferIndex = 0;
static bool buffersInitialized = false;

void sensorsManagerInit()
{
    // Configuration des pins en mode analogique (par défaut sur ESP32)
    pinMode(MQ135_PIN, INPUT);
    pinMode(MQ136_PIN, INPUT);
    pinMode(MQ4_PIN, INPUT);
    pinMode(MAX4466_PIN, INPUT);

    // Configuration de la résolution ADC (12 bits par défaut sur ESP32)
    analogReadResolution(12);

    Serial.println("Sensors Manager: Initialisation des capteurs...");

    // Temps de chauffe initial pour les capteurs MQ (recommandé: 20-48h pour une calibration optimale)
    Serial.println("Sensors Manager: Temps de chauffe des capteurs MQ en cours...");

    // Initialisation des buffers avec les premières lectures
    for (int i = 0; i < 5; i++)
    {
        mq135Buffer[i] = analogRead(MQ135_PIN);
        mq136Buffer[i] = analogRead(MQ136_PIN);
        mq4Buffer[i] = analogRead(MQ4_PIN);
        max4466Buffer[i] = analogRead(MAX4466_PIN);
        delay(50);
    }
    buffersInitialized = true;

    Serial.println("Sensors Manager: Initialisation terminée");
}

void sensorsManagerProcess()
{
    unsigned long currentTime = millis();

    // Lecture périodique pour maintenir les buffers à jour
    if (currentTime - lastReadTime >= READ_INTERVAL)
    {
        lastReadTime = currentTime;

        // Mise à jour des buffers de moyennage
        bufferIndex = (bufferIndex + 1) % 5;
        mq135Buffer[bufferIndex] = analogRead(MQ135_PIN);
        mq136Buffer[bufferIndex] = analogRead(MQ136_PIN);
        mq4Buffer[bufferIndex] = analogRead(MQ4_PIN);
        max4466Buffer[bufferIndex] = analogRead(MAX4466_PIN);
    }
}

uint16_t readMQ135Sensor()
{
    if (!buffersInitialized)
    {
        return analogRead(MQ135_PIN);
    }

    // Calcul de la moyenne des 5 dernières lectures
    uint32_t sum = 0;
    for (int i = 0; i < 5; i++)
    {
        sum += mq135Buffer[i];
    }
    return sum / 5;
}

uint16_t readMQ136Sensor()
{
    if (!buffersInitialized)
    {
        return analogRead(MQ136_PIN);
    }

    uint32_t sum = 0;
    for (int i = 0; i < 5; i++)
    {
        sum += mq136Buffer[i];
    }
    return sum / 5;
}

uint16_t readMQ4Sensor()
{
    if (!buffersInitialized)
    {
        return analogRead(MQ4_PIN);
    }

    uint32_t sum = 0;
    for (int i = 0; i < 5; i++)
    {
        sum += mq4Buffer[i];
    }
    return sum / 5;
}

uint16_t readMAX4466Sensor()
{
    if (!buffersInitialized)
    {
        return analogRead(MAX4466_PIN);
    }

    uint32_t sum = 0;
    for (int i = 0; i < 5; i++)
    {
        sum += max4466Buffer[i];
    }
    return sum / 5;
}

SensorData getAllSensorData()
{
    SensorData data;
    data.mq135Value = readMQ135Sensor();
    data.mq136Value = readMQ136Sensor();
    data.mq4Value = readMQ4Sensor();
    data.max4466Value = readMAX4466Sensor();
    return data;
}

// Fonctions de conversion (formules approximatives - à calibrer selon vos capteurs)
float mq135ToPPM(uint16_t analogValue)
{
    // Conversion approximative pour MQ135 (CO2)
    // Formule: PPM = 116.6020682 * ((Rs/R0) ^ -2.769034857)
    // Ici on utilise une approximation linéaire simple
    float voltage = (analogValue * 3.3) / 4095.0;
    float rs = (3.3 - voltage) / voltage * 1000; // Résistance du capteur en ohm
    float r0 = 76.63;                            // Résistance de référence (à calibrer)
    float ratio = rs / r0;

    if (ratio <= 0)
        return 0;

    // Approximation logarithmique
    return 116.6020682 * pow(ratio, -2.769034857);
}

float mq136ToPPM(uint16_t analogValue)
{
    // Conversion pour MQ136 (H2S)
    float voltage = (analogValue * 3.3) / 4095.0;
    float rs = (3.3 - voltage) / voltage * 1000;
    float r0 = 68.25; // À calibrer
    float ratio = rs / r0;

    if (ratio <= 0)
        return 0;

    // Formule approximative pour H2S
    return 30 * pow(ratio, -1.8);
}

float mq4ToPPM(uint16_t analogValue)
{
    // Conversion pour MQ4 (CH4)
    float voltage = (analogValue * 3.3) / 4095.0;
    float rs = (3.3 - voltage) / voltage * 1000;
    float r0 = 60.0; // À calibrer
    float ratio = rs / r0;

    if (ratio <= 0)
        return 0;

    // Formule approximative pour méthane
    return 1000 * pow(ratio, -2.3);
}

float max4466ToDecibels(uint16_t analogValue)
{
    // Conversion approximative pour MAX4466 en décibels
    // Le MAX4466 amplifie le signal du microphone
    float voltage = (analogValue * 3.3) / 4095.0;

    // Conversion approximative en décibels (à calibrer selon votre setup)
    // Niveau de référence: 1.65V = 0dB relatif
    float dbValue = 20 * log10(voltage / 1.65);

    // Ajustement pour obtenir des valeurs dB SPL approximatives
    return 50 + dbValue; // 50dB comme niveau de base
}