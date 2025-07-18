#include "i2s_microphone_fake.h"
#include <Arduino.h>
#include "modules/ble/ble_manager.h"

I2SMicrophoneFake::I2SMicrophoneFake(BLEManager *bleManager, int sampleRate, int recordTimeSec)
    : _bleManager(bleManager), _sampleRate(sampleRate), _recordTimeSec(recordTimeSec),
      _buffer(nullptr), _isRecording(false), _isDone(false), _bytesRecorded(0)
{
    _bufferSize = _sampleRate * _recordTimeSec; // mono, 8-bit
    _buffer = new uint8_t[_bufferSize];
}

I2SMicrophoneFake::~I2SMicrophoneFake()
{
    delete[] _buffer;
}

void I2SMicrophoneFake::startRecording()
{
    if (_isRecording)
        return;
    _isRecording = true;
    _isDone = false;
    _bytesRecorded = 0;
    Serial.println("Enregistrement démarré");
}

void I2SMicrophoneFake::stopRecording()
{
    if (!_isRecording)
        return;
    _isRecording = false;
    _isDone = true;

    _bleManager->sendAudioData(_buffer, _bytesRecorded);
}

void I2SMicrophoneFake::update()
{
    if (!_isRecording || _isDone)
        return;

    const size_t chunkSize = 64; // nombre d'échantillons générés à chaque update
    for (size_t i = 0; i < chunkSize && _bytesRecorded < _bufferSize; i++)
    {
        // Génération d'un signal fake : par exemple un simple bruit blanc avec valeur 128 (silence)
        // Ou un signal sinusoidal simple pour simuler une vraie forme d'onde :

        // Exemple sinus à 440Hz (La4) :
        float t = (float)_bytesRecorded / _sampleRate;
        float val = 128 + 50 * sin(2 * 3.14159265 * 440 * t);
        _buffer[_bytesRecorded++] = (uint8_t)val;

        Serial.print("En cours d'enregistrement : ");
        Serial.println(String(_bytesRecorded) + " sur " + String(_bufferSize));
    }

    if (_bytesRecorded >= _bufferSize)
    {
        stopRecording();
    }
}

bool I2SMicrophoneFake::isRecording() const
{
    return _isRecording;
}

bool I2SMicrophoneFake::isDone() const
{
    return _isDone;
}

const uint8_t *I2SMicrophoneFake::getAudioBuffer() const
{
    return _buffer;
}

size_t I2SMicrophoneFake::getBufferSize() const
{
    return _bufferSize;
}
