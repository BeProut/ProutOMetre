#include "modules/wifi/wifi_manager.h"
#include <WiFi.h>
#include <Arduino.h>
#include "config/config.h"

void connectToWiFi()
{
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connexion au WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnecté au WiFi");
}

bool isWiFiConnected()
{
    return WiFi.status() == WL_CONNECTED;
}
