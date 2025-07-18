#ifndef I2S_MICROPHONE_FAKE_H
#define I2S_MICROPHONE_FAKE_H

#include <Arduino.h>

class BLEManager;

class I2SMicrophoneFake
{
public:
    I2SMicrophoneFake(BLEManager *bleManager, int sampleRate = 16000, int recordTimeSec = 5);
    ~I2SMicrophoneFake();

    void startRecording();
    void stopRecording();
    void update();

    bool isRecording() const;
    bool isDone() const;

    const uint8_t *getAudioBuffer() const;
    size_t getBufferSize() const;

private:
    BLEManager *_bleManager;

    int _sampleRate;
    int _recordTimeSec;
    size_t _bufferSize;
    uint8_t *_buffer;

    bool _isRecording;
    bool _isDone;
    size_t _bytesRecorded;
};

#endif
