#include "fart_detector.h"

FartDetector::FartDetector()
{
    // Baselines ajustées selon les données reçues
    mq135_baseline = 370;  // Moyenne des valeurs observées
    mq136_baseline = 1970; // Moyenne des valeurs observées
    mq4_baseline = 190;    // Moyenne des valeurs observées

    mq135_threshold = 50;
    mq136_threshold = 80;
    mq4_threshold = 100;

    mq135_filtered = 0;
    mq136_filtered = 0;
    mq4_filtered = 0;

    alpha = 0.3;
}

void FartDetector::init()
{
    Serial.println("FartDetector: Initialisation...");
}

FartDetector::DetectionResult FartDetector::analyze(int mq135, int mq136, int mq4)
{
    // Filtrage passe-bas
    mq135_filtered = alpha * mq135 + (1 - alpha) * mq135_filtered;
    mq136_filtered = alpha * mq136 + (1 - alpha) * mq136_filtered;
    mq4_filtered = alpha * mq4 + (1 - alpha) * mq4_filtered;

    // Calcul des deltas
    int delta135 = max(0, (int)mq135_filtered - mq135_baseline);
    int delta136 = max(0, (int)mq136_filtered - mq136_baseline);
    int delta4 = max(0, (int)mq4_filtered - mq4_baseline);

    // Scores individuels (0-100)
    int score135 = map(constrain(delta135, 0, 500), 0, 500, 0, 100);
    int score136 = map(constrain(delta136, 0, 400), 0, 400, 0, 100);
    int score4 = map(constrain(delta4, 0, 300), 0, 300, 0, 100);

    // Score olfactif pondéré
    float smellScore = (score135 * 0.3 + score136 * 0.5 + score4 * 0.4);

    // Création du résultat
    DetectionResult result;
    result.totalScore = constrain(smellScore, 0, 100);
    result.smellFactor = result.totalScore;
    result.timestamp = millis();
    result.isDetected = (result.totalScore > 15);

    // Classification de l'intensité
    if (result.totalScore < 20)
    {
        result.intensity = 1;
        result.category = "Faible";
    }
    else if (result.totalScore < 40)
    {
        result.intensity = 2;
        result.category = "Leger";
    }
    else if (result.totalScore < 60)
    {
        result.intensity = 3;
        result.category = "Modere";
    }
    else if (result.totalScore < 80)
    {
        result.intensity = 4;
        result.category = "Fort";
    }
    else
    {
        result.intensity = 5;
        result.category = "Intense";
    }

    // Classification par type dominant
    if (score136 > score135 && score136 > score4)
    {
        result.category += " H2S";
    }
    else if (score135 > score4)
    {
        result.category += " NH3";
    }
    else if (score4 > 30)
    {
        result.category += " CH4";
    }

    return result;
}

void FartDetector::printDebug(DetectionResult result)
{
    if (result.isDetected)
    {
        Serial.print("Detection - Score: ");
        Serial.print(result.totalScore);
        Serial.print("/100, Intensite: ");
        Serial.print(result.intensity);
        Serial.print("/5, Type: ");
        Serial.println(result.category);
    }
}