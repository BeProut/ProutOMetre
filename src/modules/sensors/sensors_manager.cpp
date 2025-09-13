#include "sensors_manager.h"
#include "../../config/pins_config.h"

// ----------------------------
// CONFIGURATION
// ----------------------------
#define DEBUG true
static unsigned long lastReadTime = 0;
static const unsigned long READ_INTERVAL = 100; // ms

// Variables globales pour R0 calibré
float r0_mq135 = 76.63;
float r0_mq136 = 68.25;
float r0_mq4 = 60.0;

// ----------------------------
// INITIALISATION
// ----------------------------
void sensorsManagerInit()
{
    pinMode(MQ135_PIN, INPUT);
    pinMode(MQ136_PIN, INPUT);
    pinMode(MQ4_PIN, INPUT);
    pinMode(MAX4466_PIN, INPUT);

    Serial.println("Sensors Manager: Initialisation des capteurs...");

    // Calibration automatique
    r0_mq135 = calibrateMQ135(50);
    r0_mq136 = calibrateMQ136(50);
    r0_mq4 = calibrateMQ4(50);

    Serial.println("Sensors Manager: Initialisation terminée");
}

// ----------------------------
// CALIBRATION
// ----------------------------
float calibrateMQ135(int nSamples = 50)
{
    Serial.println("Calibrating MQ135...");
    float sumRS = 0;
    for (int i = 0; i < nSamples; i++)
    {
        uint16_t adc = analogRead(MQ135_PIN);
        float voltage = (adc * 3.3) / 4095.0;
        if (voltage < 0.01)
            voltage = 0.01;
        float rs = (3.3 - voltage) / voltage * 1000.0;
        sumRS += rs;
        delay(50);
    }
    float r0 = sumRS / nSamples / 9.0; // 9 = facteur pour ~400ppm CO2
    Serial.print("MQ135 calibrated R0 = ");
    Serial.println(r0, 2);
    return r0;
}

float calibrateMQ136(int nSamples = 50)
{
    Serial.println("Calibrating MQ136...");
    float sumRS = 0;
    for (int i = 0; i < nSamples; i++)
    {
        uint16_t adc = analogRead(MQ136_PIN);
        float voltage = (adc * 3.3) / 4095.0;
        if (voltage < 0.01)
            voltage = 0.01;
        float rs = (3.3 - voltage) / voltage * 1000.0;
        sumRS += rs;
        delay(50);
    }
    float r0 = sumRS / nSamples / 3.0; // ajuster selon datasheet (~30 ppm)
    Serial.print("MQ136 calibrated R0 = ");
    Serial.println(r0, 2);
    return r0;
}

float calibrateMQ4(int nSamples = 50)
{
    Serial.println("Calibrating MQ4...");
    float sumRS = 0;
    for (int i = 0; i < nSamples; i++)
    {
        uint16_t adc = analogRead(MQ4_PIN);
        float voltage = (adc * 3.3) / 4095.0;
        if (voltage < 0.01)
            voltage = 0.01;
        float rs = (3.3 - voltage) / voltage * 1000.0;
        sumRS += rs;
        delay(50);
    }
    float r0 = sumRS / nSamples / 4.0; // ajuster selon datasheet (~1000 ppm)
    Serial.print("MQ4 calibrated R0 = ");
    Serial.println(r0, 2);
    return r0;
}

// ----------------------------
// LECTURE PERIODIQUE
// ----------------------------
void sensorsManagerProcess()
{
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime >= READ_INTERVAL)
    {
        lastReadTime = currentTime;

        uint16_t mq135Value = analogRead(MQ135_PIN);
        uint16_t mq136Value = analogRead(MQ136_PIN);
        uint16_t mq4Value = analogRead(MQ4_PIN);
        uint16_t max4466Value = analogRead(MAX4466_PIN);

        float mq135PPM = mq135ToPPM(mq135Value);
        float mq136PPM = mq136ToPPM(mq136Value);
        float mq4PPM = mq4ToPPM(mq4Value);
        float max4466DB = max4466ToDecibels(max4466Value);

        if (DEBUG)
        {
            Serial.print("[MQ135] ADC=");
            Serial.print(mq135Value);
            Serial.print(" -> PPM=");
            Serial.println(mq135PPM, 2);
            Serial.print("[MQ136] ADC=");
            Serial.print(mq136Value);
            Serial.print(" -> PPM=");
            Serial.println(mq136PPM, 2);
            Serial.print("[MQ4]   ADC=");
            Serial.print(mq4Value);
            Serial.print(" -> PPM=");
            Serial.println(mq4PPM, 2);
            Serial.print("[MAX4466] ADC=");
            Serial.print(max4466Value);
            Serial.print(" -> dB=");
            Serial.println(max4466DB, 2);
            Serial.println("------------------------------------------------");
        }
    }
}

// ----------------------------
// CONVERSION
// ----------------------------
float mq135ToPPM(uint16_t analogValue)
{
    float voltage = (analogValue * 3.3) / 4095.0;
    if (voltage < 0.01)
        voltage = 0.01;
    float rs = (3.3 - voltage) / voltage * 1000.0;
    float ratio = rs / r0_mq135;
    return (ratio > 0) ? 116.6020682 * pow(ratio, -2.769034857) : 0;
}

float mq136ToPPM(uint16_t analogValue)
{
    float voltage = (analogValue * 3.3) / 4095.0;
    if (voltage < 0.01)
        voltage = 0.01;
    float rs = (3.3 - voltage) / voltage * 1000.0;
    float ratio = rs / r0_mq136;
    return (ratio > 0) ? 30 * pow(ratio, -1.8) : 0;
}

float mq4ToPPM(uint16_t analogValue)
{
    float voltage = (analogValue * 3.3) / 4095.0;
    if (voltage < 0.01)
        voltage = 0.01;
    float rs = (3.3 - voltage) / voltage * 1000.0;
    float ratio = rs / r0_mq4;
    return (ratio > 0) ? 1000 * pow(ratio, -2.3) : 0;
}

float max4466ToDecibels(uint16_t analogValue)
{
    float voltage = (analogValue * 3.3) / 4095.0;
    if (voltage < 0.001)
        voltage = 0.001;
    float dbValue = 20 * log10(voltage / 1.65);
    return 50 + dbValue;
}

SensorData getAllSensorData()
{
    SensorData data;
    data.mq135Value = 0;
    data.mq136Value = 0;
    data.mq4Value = 0;
    data.max4466Value = 0;
    return data;
}