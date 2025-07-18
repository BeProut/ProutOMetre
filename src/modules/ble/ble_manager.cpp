#include "ble_manager.h"
#include <BLE2902.h>
#include <Arduino.h>
#include <ArduinoJson.h>

const char *BLEManager::SERVICE_UUID = "12345678-1234-1234-1234-123456789abc";
const char *BLEManager::CHARACTERISTIC_UUID = "abcd1234-5678-90ab-cdef-1234567890ab";

BLEManager::BLEManager(LedController &led, const char *deviceName)
    : _deviceName(deviceName), _led(led), _pServer(nullptr), _pCharacteristic(nullptr),
      _deviceConnected(false), _advertising(false) {}

void BLEManager::begin()
{
    BLEDevice::init(_deviceName);
    _pServer = BLEDevice::createServer();
    _pServer->setCallbacks(new ServerCallbacks(this));

    BLEService *pService = _pServer->createService(SERVICE_UUID);

    _pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_NOTIFY);

    _pCharacteristic->addDescriptor(new BLE2902());

    pService->start();
    startAdvertising();
    Serial.println("BLE server started, waiting for clients to connect...");
}

void BLEManager::notifyUpdateState(
    int mq135,
    int mq136,
    int mq4,
    int micAnalog,
    int micLevel,
    bool buttonPressed)
{
    if (_deviceConnected && _pCharacteristic)
    {
        StaticJsonDocument<200> doc;
        doc["mq135"] = mq135;
        doc["mq136"] = mq136;
        doc["mq4"] = mq4;
        doc["micAnalog"] = micAnalog;
        doc["micLevel"] = micLevel;
        doc["buttonPressed"] = buttonPressed;

        char buffer[256];
        size_t len = serializeJson(doc, buffer);

        _pCharacteristic->setValue((uint8_t *)buffer, len);
        _pCharacteristic->notify();

        Serial.print("Notification envoyée : ");
        Serial.println(buffer);
    }
}

bool BLEManager::isConnected() const
{
    return _deviceConnected;
}

void BLEManager::loop()
{
    if (!_deviceConnected && !_advertising)
    {
        startAdvertising();
    }
}

void BLEManager::startAdvertising()
{
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    _advertising = true;
    Serial.println("Publicité BLE démarrée");
}

// ---- Callbacks ----

BLEManager::ServerCallbacks::ServerCallbacks(BLEManager *parent) : _parent(parent) {}

void BLEManager::ServerCallbacks::onConnect(BLEServer *pServer)
{
    _parent->_deviceConnected = true;
    _parent->_advertising = false;

    Serial.println("Client connecté");
    _parent->updateLed();
}

void BLEManager::ServerCallbacks::onDisconnect(BLEServer *pServer)
{
    _parent->_deviceConnected = false;
    Serial.println("Client déconnecté");
    _parent->startAdvertising();
    _parent->updateLed();
}

void BLEManager::updateLed()
{
    if (_deviceConnected)
    {
        _led.set(LED_BLINK);
    }
    else
    {
        _led.set(LED_BLINK_SOS);
    }
}
