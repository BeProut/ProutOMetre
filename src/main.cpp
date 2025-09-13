// ...existing code...
#include <Arduino.h>

#include "modules/wifi/wifi_manager.h"
#include "modules/audio/audio_recorder.h"

// Instance du gestionnaire audio
AudioRecorder audioRecorder;

void setup()
{
  Serial.begin(115200);
  Serial.print("Firmware version: ");

  // Initialiser le WiFi
  wifiManagerInit();

  // Initialiser le module audio
  audioRecorder.begin();
}

void loop()
{
  wifiManagerProcess();

  // Traiter l'enregistrement audio
  audioRecorder.process();

  delay(10); // Réduire le délai pour une meilleure réactivité du bouton
}