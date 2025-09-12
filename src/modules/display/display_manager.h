#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "modules/fart_detector/fart_detector.h"

class DisplayManager
{
private:
    Adafruit_SSD1306 *display;

    // Configuration écran
    static const int SCREEN_WIDTH = 128;
    static const int SCREEN_HEIGHT = 64;
    static const int OLED_RESET = -1;
    static const uint8_t I2C_ADDRESS = 0x3C;

    // Variables d'affichage
    unsigned long lastUpdate;
    int animationFrame;
    bool screenOn;

    // Méthodes privées d'affichage
    void drawProgressBar(int x, int y, int width, int height, int value, int maxValue);
    void drawScore(int score);
    void drawIntensity(int intensity);
    void drawCategory(String category);
    void drawHeader();
    void drawNoDetection();

public:
    DisplayManager();
    bool init();
    void update(FartDetector::DetectionResult result);
    void clear();
    void turnOn();
    void turnOff();
    void setBrightness(int level); // 0-255
    void showCalibration();
    void showStatus(String message);
};

#endif