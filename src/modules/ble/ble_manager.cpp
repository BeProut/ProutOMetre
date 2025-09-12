#include "ble_manager.h"
#include <BLE2902.h>
#include <Arduino.h>
#include <ArduinoJson.h>

BLEManager::BLEManager(LedController &led, const std::string &deviceName)
    : _led(led), _deviceName(deviceName) {}

void BLEManager::begin()
{
    BLEDevice::init(_deviceName.c_str());
    _pServer = BLEDevice::createServer();
    _pServer->setCallbacks(new ServerCallbacks(this));

    BLEService *pService = _pServer->createService(SERVICE_UUID);

    _pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_NOTIFY);
    _pCharacteristic->addDescriptor(new BLE2902());

    _pDeviceNameCharacteristic = pService->createCharacteristic(
        DEVICE_NAME_UUID,
        BLECharacteristic::PROPERTY_WRITE);
    _pDeviceNameCharacteristic->setCallbacks(new CharacteristicCallbacks(this));

    pService->start();

    startAdvertising();

    Serial.println("BLE server started, waiting for clients to connect...");
}

void BLEManager::notifyUpdateState(int mq135, int mq136, int mq4, int max4466, int micAnalog, int micLevel, bool buttonPressed)
{
    if (_deviceConnected && _pCharacteristic)
    {
        StaticJsonDocument<300> doc;
        doc["mq135"] = mq135;
        doc["mq136"] = mq136;
        doc["mq4"] = mq4;
        doc["micAnalog"] = micAnalog;
        doc["micLevel"] = micLevel;
        doc["buttonPressed"] = buttonPressed;
        doc["clientName"] = _clientDeviceName;

        char buffer[350];
        size_t len = serializeJson(doc, buffer);

        _pCharacteristic->setValue((uint8_t *)buffer, len);
        _pCharacteristic->notify();
    }
}

bool BLEManager::isConnected() const
{
    return _deviceConnected;
}

std::string BLEManager::getClientDeviceName() const
{
    return _clientDeviceName;
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

// --- Callbacks ---

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
    _parent->_clientDeviceName.clear();

    Serial.println("Client déconnecté");
    _parent->startAdvertising();
    _parent->updateLed();
}

BLEManager::CharacteristicCallbacks::CharacteristicCallbacks(BLEManager *parent) : _parent(parent) {}

void BLEManager::CharacteristicCallbacks::onWrite(BLECharacteristic *pCharacteristic)
{
    if (pCharacteristic->getUUID().toString() == DEVICE_NAME_UUID)
    {
        std::string value = pCharacteristic->getValue();
        _parent->_clientDeviceName = value;

        Serial.print("Nom du device client reçu : ");
        Serial.println(_parent->_clientDeviceName.c_str());
    }
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
