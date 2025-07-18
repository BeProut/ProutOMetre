#include "sensor_handler.h"

SensorHandler::SensorHandler(uint8_t pin, bool fake_data)
    : _pin(pin), _fake_data(fake_data)
{
}

void SensorHandler::begin()
{
    if (_fake_data)
    {
        Serial.println("Fake data mode enabled for sensor on pin " + String(_pin));
    }
    else
    {
        Serial.println("Real data mode enabled for sensor on pin " + String(_pin));
        pinMode(_pin, INPUT);
    }
}

int SensorHandler::read()
{
    if (_fake_data)
    {
        // Retourne une valeur aléatoire entre 0 et 1023 (comme une entrée analogique)
        return random(0, 1024);
    }
    else
    {
        // Lecture réelle du pin analogique (si c’est un pin analogique)
        // sinon analogRead() ou digitalRead() selon besoin
        // Ici j'assume analogRead(), adapte si besoin.
        return analogRead(_pin);
    }
}
