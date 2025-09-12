#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEAdvertising.h>
#include "modules/led/led_controller.h"
#include <string>

class BLEManager
{
public:
    explicit BLEManager(LedController &led, const std::string &deviceName = "ProutOMetre");

    void begin();
    void notifyUpdateState(int mq135, int mq136, int mq4, int max4466, int micAnalog, int micLevel, bool buttonPressed);
    bool isConnected() const;
    std::string getClientDeviceName() const;
    void loop();

private:
    static constexpr const char *SERVICE_UUID = "12345678-1234-1234-1234-123456789abc";
    static constexpr const char *CHARACTERISTIC_UUID = "abcd1234-5678-90ab-cdef-1234567890ab";
    static constexpr const char *DEVICE_NAME_UUID = "efab5678-9abc-def0-1234-567890abcdef";

    std::string _deviceName;
    LedController &_led;

    BLEServer *_pServer = nullptr;
    BLECharacteristic *_pCharacteristic = nullptr;
    BLECharacteristic *_pDeviceNameCharacteristic = nullptr;

    bool _deviceConnected = false;
    bool _advertising = false;
    std::string _clientDeviceName;

    void startAdvertising();
    void updateLed();

    // Callbacks imbriqués
    class ServerCallbacks : public BLEServerCallbacks
    {
    public:
        explicit ServerCallbacks(BLEManager *parent);
        void onConnect(BLEServer *pServer) override;
        void onDisconnect(BLEServer *pServer) override;

    private:
        BLEManager *_parent;
    };

    class CharacteristicCallbacks : public BLECharacteristicCallbacks
    {
    public:
        explicit CharacteristicCallbacks(BLEManager *parent);
        void onWrite(BLECharacteristic *pCharacteristic) override;

    private:
        BLEManager *_parent;
    };
};

#endif
