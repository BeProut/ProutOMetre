#include "uuid_manager.h"
#include <Preferences.h>
#include <esp_random.h>

static Preferences preferences;
static String currentUUID = "";

String getUUID()
{
    if (currentUUID != "")
        return currentUUID;
    preferences.begin("config", false);
    currentUUID = preferences.getString("uuid", "");
    if (currentUUID.length() == 0)
    {
        currentUUID = generateUUIDv4();
        preferences.putString("uuid", currentUUID);
        Serial.println("Nouveau UUID v4 généré : " + currentUUID);
    }
    else
    {
        Serial.println("UUID existant : " + currentUUID);
    }
    preferences.end();
    return currentUUID;
}

String generateUUIDv4()
{
    uint8_t uuid[16];
    for (int i = 0; i < 16; i++)
        uuid[i] = esp_random() & 0xFF;

    uuid[6] = (uuid[6] & 0x0F) | 0x40;
    uuid[8] = (uuid[8] & 0x3F) | 0x80;

    char uuid_str[37];
    snprintf(uuid_str, sizeof(uuid_str),
             "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X",
             uuid[0], uuid[1], uuid[2], uuid[3],
             uuid[4], uuid[5],
             uuid[6], uuid[7],
             uuid[8], uuid[9],
             uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);

    return String(uuid_str);
}
