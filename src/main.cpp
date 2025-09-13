// ...existing code...
#include <Arduino.h>
#include <version.h>

#include "modules/wifi/wifi_manager.h"
#include "modules/screen/screen_manager.h"
#include "modules/heartbeat/heartbeat_manager.h"
#include "modules/uuid/uuid_manager.h"
#include "modules/ota/ota_manager.h"
#include "modules/sensors/sensors_manager.h"
#include "modules/sensors/sensor_buffer.h"

// ...existing code...

void setup()
{
  Serial.begin(115200);
  Serial.print("Firmware version: ");
  Serial.println(VERSION);
  sensorsManagerInit();
  sensorBufferInit();
  screenManagerInit();
  wifiManagerInit();
  heartbeatManagerInit();
  otaManagerInit();
}

void loop()
{
  wifiManagerProcess();
  screenManagerProcess();
  sensorBufferProcess(); // Prend un échantillon toutes les 100ms si nécessaire
  otaManagerHandle();
  delay(50);
}