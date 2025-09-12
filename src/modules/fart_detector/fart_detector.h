#ifndef FART_DETECTOR_H
#define FART_DETECTOR_H

#include <Arduino.h>

class FartDetector
{
private:
    // Valeurs de référence (air propre)
    int mq135_baseline;
    int mq136_baseline;
    int mq4_baseline;

    // Seuils de détection
    int mq135_threshold;
    int mq136_threshold;
    int mq4_threshold;

    // Historique pour filtrage
    float mq135_filtered;
    float mq136_filtered;
    float mq4_filtered;

    // Constante de filtrage
    float alpha;

public:
    struct DetectionResult
    {
        int totalScore;  // Score total 0-100
        int intensity;   // Intensité 1-5
        int smellFactor; // Score olfactif
        String category; // Type détecté
        unsigned long timestamp;
        bool isDetected; // Seuil de détection atteint
    };

    FartDetector();
    void init();
    DetectionResult analyze(int mq135, int mq136, int mq4);
    void printDebug(DetectionResult result);
};

#endif