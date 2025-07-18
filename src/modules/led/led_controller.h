#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h>

enum LedStatus
{
    LED_OFF,
    LED_ON,
    LED_BLINK,
    LED_BLINK_SOS
};

class LedController
{
public:
    void begin(uint8_t pin);
    void set(LedStatus status);
    void update(); // à appeler régulièrement dans loop()

private:
    uint8_t _pin;
    LedStatus _status = LED_OFF;
    bool _ledState = false;
    unsigned long _lastChange = 0;

    // Blink simple
    unsigned long _blinkInterval = 500;

    // Blink SOS
    int _sosStep = 0;
    unsigned long _sosPattern[8] = {100, 100, 100, 400, 100, 100, 100, 1000}; // 3 flashs, pause, 3 flashs, pause
};

#endif
