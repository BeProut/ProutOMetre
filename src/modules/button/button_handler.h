#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

class ButtonHandler
{
public:
    // Constructeur : prend référence au BLEManager
    ButtonHandler(uint8_t buttonPin);

    // Initialisation (pinMode, etc)
    void begin();

    // À appeler dans loop() pour vérifier le bouton
    bool checkButtonChange();

private:
    uint8_t _buttonPin;
    int _lastButtonState;
    int _countButtonPresses;
};

#endif // BUTTON_HANDLER_H
