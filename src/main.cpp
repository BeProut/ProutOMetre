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

// Instances des modules
UUIDManager uuidManager;
LedController led;
DisplayManager displayManager;
BLEManager bleManager(led, displayManager);

ButtonHandler buttonHandler(BUTTON_PIN);
SensorHandler mq135Sensor(MQ135_PIN, true);
SensorHandler mq136Sensor(MQ136_PIN, true);
SensorHandler mq4Sensor(MQ4_PIN, true);
SensorHandler max4466Sensor(MAX4466_PIN, true);

void setup()
{
  Serial.begin(115200);
  Serial.println("Démarrage du système...");
  Serial.printf("Firmware version: %d\n", FIRMWARE_VERSION);

  // Initialisation de l'écran
  if (!displayManager.begin())
  {
    Serial.println("Erreur: Impossible d'initialiser l'écran");
    for (;;)
      ; // Arrêt en cas d'erreur critique
  }

  // Configuration des informations à afficher
  displayManager.setFirmwareVersion(String(FIRMWARE_VERSION));

  // Initialisation des autres modules
  uuidManager.init();

  led.begin(LED_PIN);
  led.set(LED_BLINK_SOS);
  displayManager.setStatus(DISPLAY_STATUS_WAITING_CONNECTION);

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

  // Mise à jour de l'affichage
  displayManager.update();

  // Gestion du changement de statut BLE
  static bool wasConnected = false;
  bool isConnected = bleManager.isConnected();

  // Envoi des données de capteurs
  static unsigned long lastSend = 0;
  bool buttonPressed = buttonHandler.checkButtonChange();

  if (isConnected && millis() - lastSend >= 500)
  {
    lastSend = millis();
    int mq135 = mq135Sensor.read();
    int mq136 = mq136Sensor.read();
    int mq4 = mq4Sensor.read();
    int max4466 = max4466Sensor.read();
    int micAnalog = random(0, 100);
    int micLevel = random(0, 100);

    bleManager.notifyUpdateState(mq135, mq136, mq4, max4466, micAnalog, micLevel, buttonPressed);
  }

  delay(100);
}