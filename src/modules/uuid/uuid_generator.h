#ifndef UUIDMANAGER_H
#define UUIDMANAGER_H

#include <Preferences.h>
#include <Arduino.h> // pour String

class UUIDManager
{
public:
    void init();
    String getUUID();

private:
    Preferences preferences;
    String _uuid;

    String generateUUIDv4();
};

#endif
