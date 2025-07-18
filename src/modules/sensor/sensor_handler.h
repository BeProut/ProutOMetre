#ifndef SENSORHANDLER_H
#define SENSORHANDLER_H

#include <Arduino.h>

class SensorHandler
{
public:
    // Constructeur : pin à lire + fake_data (par défaut false)
    SensorHandler(uint8_t pin, bool fake_data = false);

    // Initialisation du capteur (mode pin, etc)
    void begin();

    // Lecture de la valeur (réelle ou fake selon fake_data)
    int read();

private:
    uint8_t _pin;
    bool _fake_data;
};

#endif // SENSORHANDLER_H
