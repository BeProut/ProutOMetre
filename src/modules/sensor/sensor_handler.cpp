#include "sensor_handler.h"

SensorHandler::SensorHandler(uint8_t pin, bool fake_data, SensorType type)
    : _pin(pin), _fake_data(fake_data), _sensor_type(type), _last_value(0), _last_update(0)
{
    // Initialisation selon le type de capteur
    switch (_sensor_type)
    {
    case TEMPERATURE:
        _base_value = 2048; // ~20°C en valeur ADC (exemple)
        _noise_amplitude = 50;
        _drift_speed = 0.02f;
        break;
    case HUMIDITY:
        _base_value = 1638; // ~60% d'humidité
        _noise_amplitude = 80;
        _drift_speed = 0.01f;
        break;
    case LIGHT:
        _base_value = 1024; // Luminosité moyenne
        _noise_amplitude = 200;
        _drift_speed = 0.05f;
        break;
    case PRESSURE:
        _base_value = 2048; // Pression atmosphérique normale
        _noise_amplitude = 30;
        _drift_speed = 0.005f;
        break;
    default:
        _base_value = 2048;
        _noise_amplitude = 100;
        _drift_speed = 0.02f;
        break;
    }

    _last_value = _base_value;
    _trend = 0.0f;
}

void SensorHandler::begin()
{
    if (_fake_data)
    {
        const char *type_names[] = {"Generic", "Temperature", "Humidity", "Light", "Pressure"};
        Serial.println("Fake data mode enabled for " + String(type_names[_sensor_type]) +
                       " sensor on pin " + String(_pin));
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
        return generateRealisticValue();
    }
    else
    {
        return analogRead(_pin);
    }
}

int SensorHandler::generateRealisticValue()
{
    unsigned long current_time = millis();

    // Mise à jour seulement si assez de temps s'est écoulé (évite les valeurs trop erratiques)
    if (current_time - _last_update < 100)
    {
        return _last_value + random(-2, 3); // Petite variation pour simuler le bruit
    }

    _last_update = current_time;

    // Génération d'une tendance lente (dérive naturelle)
    if (random(0, 100) < 5)
    {                                         // 5% de chance de changer de tendance
        _trend = random(-100, 101) / 1000.0f; // Nouvelle tendance entre -0.1 et 0.1
    }

    // Application de la tendance
    _base_value += _trend;

    // Bruit aléatoire autour de la valeur de base
    int noise = random(-_noise_amplitude, _noise_amplitude + 1);
    int new_value = _base_value + noise;

    // Simulation d'événements occasionnels selon le type de capteur
    simulateEvents();

    // Contraindre dans la plage valide (0-4095 pour 12-bit ADC)
    new_value = constrain(new_value, 0, 4095);

    // Lissage pour éviter les changements trop brusques
    _last_value = (_last_value * 0.7f) + (new_value * 0.3f);

    return (int)_last_value;
}

void SensorHandler::simulateEvents()
{
    // Simulation d'événements spécifiques selon le type de capteur
    switch (_sensor_type)
    {
    case TEMPERATURE:
        // Simulation de changements thermiques graduels
        if (random(0, 1000) < 2)
        {                                     // 0.2% de chance
            _base_value += random(-200, 201); // Changement de température
        }
        break;

    case HUMIDITY:
        // Simulation de variations d'humidité
        if (random(0, 1000) < 5)
        { // 0.5% de chance
            _base_value += random(-300, 301);
        }
        break;

    case LIGHT:
        // Simulation de changements d'éclairage plus fréquents
        if (random(0, 1000) < 10)
        { // 1% de chance
            _base_value += random(-500, 501);
        }
        // Contraindre pour la lumière (ne peut pas être négatif)
        _base_value = max(_base_value, 0.0f);

        break;

    case PRESSURE:
        // La pression varie très lentement
        if (random(0, 1000) < 1)
        { // 0.1% de chance
            _base_value += random(-50, 51);
        }
        break;
    }

    // Maintenir la valeur de base dans une plage raisonnable
    _base_value = constrain(_base_value, 0, 4095);
}

void SensorHandler::setSensorType(SensorType type)
{
    _sensor_type = type;
}

float SensorHandler::getLastValueAsFloat()
{
    // Conversion en valeur physique approximative selon le type
    switch (_sensor_type)
    {
    case TEMPERATURE:
        return (_last_value / 4095.0f) * 100.0f - 10.0f; // -10°C à 90°C
    case HUMIDITY:
        return (_last_value / 4095.0f) * 100.0f; // 0% à 100%
    case LIGHT:
        return (_last_value / 4095.0f) * 1000.0f; // 0 à 1000 lux
    case PRESSURE:
        return 950.0f + (_last_value / 4095.0f) * 100.0f; // 950 à 1050 hPa
    default:
        return _last_value;
    }
}