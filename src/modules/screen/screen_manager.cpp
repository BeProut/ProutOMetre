#include "screen_manager.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "modules/wifi/wifi_manager.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define I2C_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void screenManagerInit()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ;
    }
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Demarrage...");
    display.display();
}

void screenManagerProcess()
{
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    if (wifiManagerIsConnected())
    {
        display.println("WiFi OK");
    }
    else
    {
        display.println("WiFi OFF");
    }
    display.display();
}
