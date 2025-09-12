#ifndef HEARTBEAT_MANAGER_H
#define HEARTBEAT_MANAGER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// Structure pour stocker les données reçues du serveur (thread-safe)
struct HeartbeatData
{
    String version;
    bool debug;
    bool debugExists;
    bool lastRequestSuccess;
    unsigned long lastRequestTime;
};

// Fonctions publiques du module heartbeat
void heartbeatManagerInit();

#endif // HEARTBEAT_MANAGER_H