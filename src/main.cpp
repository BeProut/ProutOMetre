#include <Arduino.h>
#include <Preferences.h>
#include "modules/wifi/wifi_manager.h"
#include "modules/uuid/uuid_generator.h"
#include "modules/button/button_handler.h"
#include "config/config.h"
#include "version.h"

Preferences preferences;

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Démarrage du système...");
  Serial.printf("Firmware version: %d\n", FIRMWARE_VERSION);

  initButton();
  connectToWiFi();

  preferences.begin("config", false);
  String uuid = preferences.getString("uuid", "");
  if (uuid.length() == 0)
  {
    uuid = generateUUIDv4();
    preferences.putString("uuid", uuid);
    Serial.println("Nouveau UUID v4 généré : " + uuid);
  }
  else
  {
    Serial.println("UUID existant : " + uuid);
  }
  preferences.end();
}

void loop()
{
  checkButtonChange();
  delay(100);
}
