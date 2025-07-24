#ifndef SENSOR_HANDLER_H
#define SENSOR_HANDLER_H

#include <Arduino.h>

enum SensorType
{
    GENERIC = 0,
    TEMPERATURE = 1,
    HUMIDITY = 2,
    LIGHT = 3,
    PRESSURE = 4
};

class SensorHandler
{
private:
    uint8_t _pin;
    bool _fake_data;
    SensorType _sensor_type;

    // Variables pour la simulation réaliste
    float _base_value;          // Valeur de base du capteur
    int _noise_amplitude;       // Amplitude du bruit
    float _drift_speed;         // Vitesse de dérive
    float _trend;               // Tendance actuelle
    float _last_value;          // Dernière valeur générée
    unsigned long _last_update; // Dernière mise à jour

    int generateRealisticValue();
    void simulateEvents();

public:
    SensorHandler(uint8_t pin, bool fake_data = false, SensorType type = GENERIC);

    void begin();
    int read();
    void setSensorType(SensorType type);
    float getLastValueAsFloat(); // Retourne une valeur convertie selon le type de capteur
};

#endif