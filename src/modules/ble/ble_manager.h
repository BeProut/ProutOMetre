#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "modules/led/led_controller.h"

class BLEManager
{
public:
    BLEManager(LedController &led, const char *deviceName = "ProutOMetre");
    void begin();
    void notifyUpdateState(
        int mq135,
        int mq136,
        int mq4,
        int max4466,
        int micAnalog,
        int micLevel,
        bool buttonPressed);
    bool isConnected() const;
    void loop();
    void sendAudioData(const uint8_t *data, size_t length);

private:
    class ServerCallbacks : public BLEServerCallbacks
    {
    public:
        ServerCallbacks(BLEManager *parent);
        void onConnect(BLEServer *pServer) override;
        void onDisconnect(BLEServer *pServer) override;

    private:
        BLEManager *_parent;
    };

    void startAdvertising();
    void updateLed();

    const char *_deviceName;
    BLEServer *_pServer;
    BLECharacteristic *_pCharacteristic;
    BLECharacteristic *_pAudioCharacteristic;

    bool _deviceConnected;
    bool _advertising;

    LedController &_led;

    static const char *SERVICE_UUID;
    static const char *CHARACTERISTIC_UUID;
    static const char *AUDIO_CHARACTERISTIC_UUID;
};

#endif // BLE_MANAGER_H
