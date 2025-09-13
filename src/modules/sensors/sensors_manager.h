#pragma once
#include <Arduino.h>

// Structure pour stocker les valeurs des capteurs
struct SensorData
{
    uint16_t mq135Value;   // Capteur de qualité d'air (CO2, NH3, NOx, alcool, benzène, fumée, CO)
    uint16_t mq136Value;   // Capteur de sulfure d'hydrogène (H2S)
    uint16_t mq4Value;     // Capteur de méthane (CH4) et gaz naturel
    uint16_t max4466Value; // Capteur de niveau sonore (microphone)
};

// Fonctions d'initialisation et de gestion
void sensorsManagerInit();
void sensorsManagerProcess();

// Fonctions de lecture des capteurs individuels
uint16_t readMQ135Sensor();
uint16_t readMQ136Sensor();
uint16_t readMQ4Sensor();
uint16_t readMAX4466Sensor();

// Fonction pour récupérer toutes les valeurs en une fois
SensorData getAllSensorData();

// Fonctions utilitaires pour la conversion des valeurs
float mq135ToPPM(uint16_t analogValue);
float mq136ToPPM(uint16_t analogValue);
float mq4ToPPM(uint16_t analogValue);
float max4466ToDecibels(uint16_t analogValue);

// Fonctions de calibration des capteurs
float calibrateMQ135(int nSamples);
float calibrateMQ136(int nSamples);
float calibrateMQ4(int nSamples);