#include "button_handler.h"

ButtonHandler::ButtonHandler(uint8_t buttonPin)
    : _buttonPin(buttonPin), _lastButtonState(LOW), _countButtonPresses(0) {}

void ButtonHandler::begin()
{
    pinMode(_buttonPin, INPUT);
    _lastButtonState = digitalRead(_buttonPin);
}

bool ButtonHandler::checkButtonChange()
{
    int newButtonState = digitalRead(_buttonPin);
    if (newButtonState != _lastButtonState)
    {
        _lastButtonState = newButtonState;

        // Message local série
        if (newButtonState == HIGH)
        {
            Serial.println("Start pressed");
        }
        else
        {
            Serial.println("Stop pressed");
        }
    }

    return (newButtonState == HIGH);
}
