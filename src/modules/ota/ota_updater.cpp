#include "modules/ota/ota_updater.h"
#include <HTTPClient.h>
#include <Update.h>
#include <WiFi.h>
#include <Arduino.h>
#include "config/config.h"
#include "modules/wifi/wifi_manager.h"

void checkAndUpdateFirmware()
{
    if (isWiFiConnected())
    {
        Serial.println("WiFi connecté, vérification de la mise à jour...");
    }
    else
    {
        Serial.println("WiFi non connecté, impossible de vérifier la mise à jour.");
        return;
    }
    HTTPClient http;
    http.begin(FIRMWARE_URL);

    int httpCode = http.GET();
    if (httpCode == 200)
    {
        int contentLength = http.getSize();
        WiFiClient *stream = http.getStreamPtr();

        if (Update.begin(contentLength))
        {
            Serial.println("Mise à jour OTA commencée...");
            size_t written = Update.writeStream(*stream);
            if (written == contentLength)
            {
                Serial.println("Mise à jour OTA terminée !");
                if (Update.end() && Update.isFinished())
                {
                    Serial.println("Redémarrage...");
                    ESP.restart();
                }
                else
                {
                    Serial.println("Mise à jour incomplète !");
                }
            }
            else
            {
                Serial.println("Erreur : tout le firmware n'a pas été écrit.");
            }
        }
        else
        {
            Serial.println("Erreur : Update.begin() a échoué.");
        }
    }
    else
    {
        Serial.printf("Erreur HTTP : %d\n", httpCode);
    }

    http.end();
}
