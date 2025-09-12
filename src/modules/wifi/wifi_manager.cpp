
#include "wifi_manager.h"
#include <WiFi.h>

const char *ssidList[] = {"SSID_Maison", "SSID_Bureau", "MonHotspot"};
const char *passList[] = {"mdpMaison", "mdpBureau", "mdpHotspot"};
const int nbReseaux = sizeof(ssidList) / sizeof(ssidList[0]);

static int wifiCurrentIndex = 0;
static unsigned long wifiAttemptStart = 0;
static bool wifiConnecting = false;
static unsigned long lastAllWifiCheck = 0;

// À appeler une seule fois au démarrage
void wifiManagerInit()
{
    Serial.println("Tentative de connexion au Wi-Fi...");
    WiFi.mode(WIFI_STA);
    wifiCurrentIndex = 0;
    wifiConnecting = true;
    wifiAttemptStart = 0;
}

// À appeler régulièrement dans la loop principale
void wifiManagerProcess()
{
    if (WiFi.status() == WL_CONNECTED && !wifiConnecting)
        return;

    if (!wifiConnecting && millis() - lastAllWifiCheck > 30000)
    {
        Serial.println("Nouvelle tentative de connexion aux réseaux Wi-Fi connus...");
        wifiCurrentIndex = 0;
        wifiConnecting = true;
        wifiAttemptStart = 0;
    }
    else
    {
        return;
    }

    if (wifiCurrentIndex >= nbReseaux)
    {
        Serial.println("Impossible de se connecter à un réseau après tous les essais.");
        wifiConnecting = false;
        lastAllWifiCheck = millis();
        return;
    }

    if (wifiAttemptStart == 0)
    {
        Serial.printf("Essai de connexion à %s...\n", ssidList[wifiCurrentIndex]);
        WiFi.begin(ssidList[wifiCurrentIndex], passList[wifiCurrentIndex]);
        wifiAttemptStart = millis();
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.printf("Connecté à %s\n", ssidList[wifiCurrentIndex]);
        Serial.print("Adresse IP : ");
        Serial.println(WiFi.localIP());
        wifiConnecting = false;
    }
    else if (millis() - wifiAttemptStart > 8000)
    {
        Serial.printf("Échec sur %s\n", ssidList[wifiCurrentIndex]);
        WiFi.disconnect(true);
        wifiCurrentIndex++;
        wifiAttemptStart = 0;
    }
}

bool wifiManagerIsConnected()
{
    return WiFi.status() == WL_CONNECTED;
}
