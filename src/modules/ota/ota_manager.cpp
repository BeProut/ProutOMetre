// ota_manager.cpp
#include "ota_manager.h"
#include <Arduino.h>
#include <WiFi.h>
#include <modules/wifi/wifi_manager.h>

static char otaUrl[256] = "";
static volatile bool otaRequested = false;
static volatile bool otaInProgress = false;

void otaManagerInit()
{
    // Rien à init pour HTTPUpdate
    Serial.println("OTA Manager ready");
}

void otaManagerSetUrl(const char *url)
{
    strncpy(otaUrl, url, sizeof(otaUrl) - 1);
    otaUrl[sizeof(otaUrl) - 1] = '\0'; // sécurité
    otaRequested = true;
    Serial.println("OTA requested: ");
    Serial.println(otaUrl);
}

void otaManagerHandle()
{
    if (otaRequested && !otaInProgress)
    {
        otaRequested = false;
        otaInProgress = true;

        if (!wifiManagerIsConnected())
        {
            Serial.println("WiFi not connected, cannot start OTA");
            return;
        }

        Serial.print("Starting OTA from: ");
        Serial.println(otaUrl);

        WiFiClient client;
        t_httpUpdate_return ret = httpUpdate.update(client, otaUrl);

        switch (ret)
        {
        case HTTP_UPDATE_FAILED:
            Serial.printf("OTA failed: %d %s\n",
                          httpUpdate.getLastError(),
                          httpUpdate.getLastErrorString().c_str());
            otaInProgress = false;
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("OTA no updates available");
            otaInProgress = false;
            break;
        case HTTP_UPDATE_OK:
            Serial.println("OTA success! Device will reboot automatically.");
            break;
        }
    }
}
