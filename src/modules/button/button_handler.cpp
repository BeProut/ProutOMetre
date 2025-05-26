#include "modules/button/button_handler.h"
#include <Arduino.h>
#include "modules/ota/ota_updater.h"
#include "modules/wifi/wifi_manager.h"

int BUTTON_PIN = 4; 

int buttonState = 0;

void initButton()
{
    pinMode(BUTTON_PIN, INPUT);
}

void checkButtonChange()
{
    int newButtonState = digitalRead(BUTTON_PIN);
    if (newButtonState != buttonState)
    {
        buttonState = newButtonState;
        if (buttonState == HIGH)
        {
            Serial.println("Bouton pressé, vérification de mise à jour...");
            checkAndUpdateFirmware();
        }
    }
}
