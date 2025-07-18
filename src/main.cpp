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

UUIDManager uuidManager;
LedController led;
BLEManager bleManager(led);

ButtonHandler buttonHandler(BUTTON_PIN);
SensorHandler mq135Sensor(MQ135_PIN, true);
SensorHandler mq136Sensor(MQ136_PIN, true);
SensorHandler mq4Sensor(MQ4_PIN, true);

void setup()
{
  Serial.begin(115200);
  Serial.println("Démarrage du système...");
  Serial.printf("Firmware version: %d\n", FIRMWARE_VERSION);
  uuidManager.init();
  led.begin(LED_PIN);
  led.set(LED_BLINK_SOS);
  bleManager.begin();
  buttonHandler.begin();
  mq135Sensor.begin();
  mq136Sensor.begin();
  mq4Sensor.begin();
}

void loop()
{
  led.update();
  bleManager.loop();
  static unsigned long lastSend = 0;
  if (bleManager.isConnected() && millis() - lastSend >= 500)
  {
    lastSend = millis();
    int mq135 = mq135Sensor.read();
    int mq136 = mq136Sensor.read();
    int mq4 = mq4Sensor.read();
    int micAnalog = random(0, 100);
    int micLevel = random(0, 100);
    bool buttonPressed = buttonHandler.checkButtonChange();

    bleManager.notifyUpdateState(mq135, mq136, mq4, micAnalog, micLevel, buttonPressed);
  }
  delay(100);
}
