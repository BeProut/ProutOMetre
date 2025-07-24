#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

enum DisplayStatus
{
    DISPLAY_STATUS_WAITING_CONNECTION,
    DISPLAY_STATUS_CONNECTED,
    DISPLAY_STATUS_ERROR
};

class DisplayManager
{
public:
    DisplayManager(); // 👈 constructeur par défaut
    ~DisplayManager();

    bool begin(); // 👈 plus besoin de passer d’adresse I2C

    void update();
    void setStatus(DisplayStatus status);
    void setDeviceName(const String &name);
    void setFirmwareVersion(const String &version);

    void showMessage(const String &message, int duration);
    void clear();
    void setBrightness(uint8_t brightness);

private:
    static const int SCREEN_WIDTH = 128;
    static const int SCREEN_HEIGHT = 64;
    static const int OLED_RESET = -1;
    static const uint8_t I2C_ADDRESS = 0x3C;

    Adafruit_SSD1306 *display;
    DisplayStatus currentStatus;
    unsigned long lastUpdate;
    int animationCounter;

    String deviceName;
    String firmwareVersion;

    void drawWaitingConnection();
    void drawConnectedStatus();
    void drawErrorStatus();

    void drawLoadingAnimation(int x, int y, int radius);
    void drawCenteredText(const String &text, int y, int textSize);
    void drawScrollingText(const String &text, int y, int textSize);
};

String removeAccents(const String &input);

#endif // DISPLAY_MANAGER_H
