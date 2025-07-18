#include "led_controller.h"

void LedController::begin(uint8_t pin)
{
    _pin = pin;
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void LedController::set(LedStatus status)
{
    _status = status;
    _lastChange = millis();
    _sosStep = 0;
    _ledState = (status == LED_ON);
    digitalWrite(_pin, _ledState ? HIGH : LOW);
}

void LedController::update()
{
    unsigned long currentMillis = millis();

    switch (_status)
    {
    case LED_OFF:
    case LED_ON:
        // Rien à faire, l’état est géré via `set`
        break;

    case LED_BLINK:
        if (currentMillis - _lastChange >= _blinkInterval)
        {
            _ledState = !_ledState;
            digitalWrite(_pin, _ledState ? HIGH : LOW);
            _lastChange = currentMillis;
        }
        break;

    case LED_BLINK_SOS:
        if (currentMillis - _lastChange >= _sosPattern[_sosStep])
        {
            _ledState = !_ledState;
            digitalWrite(_pin, _ledState ? HIGH : LOW);
            _lastChange = currentMillis;
            _sosStep = (_sosStep + 1) % 8; // boucle sur le pattern
        }
        break;
    }
}
