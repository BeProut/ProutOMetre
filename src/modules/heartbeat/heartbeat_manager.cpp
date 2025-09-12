#include "heartbeat_manager.h"
#include <modules/wifi/wifi_manager.h>
#include <modules/sensors/sensors_manager.h>
#include <modules/sensors/sensor_buffer.h>
#include <version.h>
#include <modules/uuid/uuid_manager.h>
#include <modules/ota/ota_manager.h>

// Configuration du heartbeat
static const char *HEARTBEAT_URL = "http://localhost:3000/prout-o-metre/heartbeat";
static const unsigned long HEARTBEAT_INTERVAL = 5000; // Envoi toutes les 5 secondes
static const unsigned long HTTP_TIMEOUT = 3000;       // Timeout de 3 secondes

static TaskHandle_t heartbeatTaskHandle = NULL;
volatile String otaUpdateUrl = "";

// Fonction de la tâche heartbeat (s'exécute en parallèle)
void heartbeatTask(void *parameter)
{
    HTTPClient http;
    bool isDebugEnabled = false;
    String deviceUUID = getUUID();

    while (true)
    {
        // Attendre l'intervalle du heartbeat
        vTaskDelay(pdMS_TO_TICKS(HEARTBEAT_INTERVAL));

        // Vérifier la connexion WiFi
        if (!wifiManagerIsConnected())
            continue;

        Serial.println("Heartbeat Task: Envoi du heartbeat...");

        // Créer le document JSON pour le payload
        JsonDocument doc;

        // Ajouter l'UUID et la version du firmware
        doc["uuid"] = deviceUUID;
        doc["firmwareVersion"] = VERSION;

        if (isDebugEnabled)
        {
            // Vérifier s'il y a des données dans le buffer
            int bufferSize = getSensorBufferSize();
            if (bufferSize > 0)
            {
                Serial.printf("Heartbeat Task: Envoi de %d échantillons de capteurs\n", bufferSize);
                // Ajouter toutes les données du buffer au JSON
                getSensorBufferJson(doc);
                // Vider le buffer après envoi
                clearSensorBuffer();
            }
            else
            {
                Serial.println("Heartbeat Task: Aucune donnée de capteur à envoyer");
            }
        }

        // Sérialiser le JSON en string
        String jsonPayload;
        serializeJson(doc, jsonPayload);

        // Préparer et envoyer la requête HTTP
        http.begin(HEARTBEAT_URL);
        http.setTimeout(HTTP_TIMEOUT);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("User-Agent", "ProutOMetre-ESP32");

        int httpCode = http.POST(jsonPayload); // Envoi de la requête POST avec le payload JSON
        unsigned long requestTime = millis();

        if (httpCode <= 0)
        {
            Serial.printf("Heartbeat Task: Connexion échouée: %s\n", http.errorToString(httpCode).c_str());
        }
        else if (httpCode != 200)
        {
            Serial.printf("Heartbeat Task: HTTP error %d\n", httpCode);
        }
        else
        {
            String response = http.getString();
            Serial.printf("Heartbeat Task: Réponse HTTP 200: %s\n", response.c_str());

            JsonDocument doc;
            if (deserializeJson(doc, response) == DeserializationError::Ok)
            {
                isDebugEnabled = doc["debug"].is<bool>() ? doc["debug"].as<bool>() : false;
                if (doc["update_firmware_url"].is<const char *>())
                {
                    otaManagerSetUrl(
                        doc["update_firmware_url"].as<const char *>());
                }
            }
            else
            {
                Serial.println("Heartbeat Task: Erreur parsing JSON");
            }
        }

        http.end();
    }
}

void heartbeatManagerInit()
{
    Serial.println("Heartbeat Manager: Initialisation du module heartbeat...");

    // Créer la tâche heartbeat (sur le core 0, priorité 1)
    BaseType_t result = xTaskCreatePinnedToCore(
        heartbeatTask,        // Fonction de la tâche
        "HeartbeatTask",      // Nom de la tâche
        4096,                 // Taille de la pile (4KB)
        NULL,                 // Paramètre de la tâche
        1,                    // Priorité (1 = basse priorité)
        &heartbeatTaskHandle, // Handle de la tâche
        0                     // Core 0 (le core 1 est pour la loop principale)
    );

    if (result == pdPASS)
    {
        Serial.println("Heartbeat Manager: Tâche heartbeat créée avec succès");
    }
    else
    {
        Serial.println("Heartbeat Manager: Erreur - Impossible de créer la tâche heartbeat");
    }

    Serial.println("Heartbeat Manager: Initialisation terminée");
}
