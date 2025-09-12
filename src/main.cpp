#include <Arduino.h>
#include <Preferences.h>
#include "modules/uuid/uuid_generator.h"
#include "modules/button/button_handler.h"
#include "config/config.h"
#include "config/pins_config.h"
#include "version.h"
#include "modules/ble/ble_manager.h"
#include <ArduinoJson.h>
#include "modules/sensor/sensor_handler.h"
#include "modules/led/led_controller.h"
#include "modules/i2s_microphone/i2s_microphone_fake.h"
#include "modules/display/display_manager.h"
#include "modules/fart_detector/fart_detector.h"

// Instances des modules
UUIDManager uuidManager;
LedController led;
DisplayManager displayManager;
BLEManager bleManager(led);
FartDetector detector;

ButtonHandler buttonHandler(BUTTON_PIN);

bool fakeData = false; // Utilisé pour simuler des données de capteurs

SensorHandler mq135Sensor(MQ135_PIN, fakeData);
SensorHandler mq136Sensor(MQ136_PIN, fakeData);
SensorHandler mq4Sensor(MQ4_PIN, fakeData);
SensorHandler max4466Sensor(MAX4466_PIN, fakeData);

void setup()
{
  Serial.begin(115200);
  Serial.println("Démarrage du système...");
  Serial.printf("Firmware version: %d\n", FIRMWARE_VERSION);

  // Initialisation des autres modules
  uuidManager.init();
  detector.init();
  if (!displayManager.init())
  {
    Serial.println("ERREUR: Impossible d'initialiser l'écran");
    while (1)
      ; // Arrêt si écran non fonctionnel
  }

  displayManager.showStatus("Initialisation...");

  led.begin(LED_PIN);
  led.set(LED_BLINK_SOS);

  bleManager.begin();
  buttonHandler.begin();
  mq135Sensor.begin();
  mq136Sensor.begin();
  mq4Sensor.begin();
  max4466Sensor.begin();

  Serial.println("Système prêt.");
}

void loop()
{
  led.update();
  bleManager.loop();

  // Gestion du changement de statut BLE
  static bool wasConnected = false;
  bool isConnected = bleManager.isConnected();

  // Envoi des données de capteurs
  static unsigned long lastSend = 0;
  bool buttonPressed = buttonHandler.checkButtonChange();

  if (millis() - lastSend >= 500)
  {
    lastSend = millis();
    int mq135 = mq135Sensor.read();
    int mq136 = mq136Sensor.read();
    int mq4 = mq4Sensor.read();
    int max4466 = max4466Sensor.read();
    int micAnalog = random(0, 100);
    int micLevel = random(0, 100);

    FartDetector::DetectionResult result = detector.analyze(mq135, mq136, mq4);

    // Mise à jour de l'affichage
    displayManager.update(result);

    // affiche moi les valeurs des capteurs
    Serial.println("MQ135: " + String(mq135) +
                   ", MQ136: " + String(mq136) +
                   ", MQ4: " + String(mq4) +
                   ", MAX4466: " + String(max4466) +
                   ", Mic Analog: " + String(micAnalog) +
                   ", Mic Level: " + String(micLevel));

    if (isConnected)
    {
      bleManager.notifyUpdateState(mq135, mq136, mq4, max4466, micAnalog, micLevel, buttonPressed);
    }
  }

  delay(100);
}