#pragma once

#include <aaudio/AAudio.h>
#include <atomic>
#include <memory>
#include <string>
#include "LockFreeRingBuffer.h"
#include "AudioDecoder.h"

class AudioEngine {
public:
    AudioEngine();

    ~AudioEngine();

    bool start();

    void stop();

    void setVolume(float volume);

    void setMute(bool muted);

    void switchStream(const std::string &url);

private:
    static aaudio_data_callback_result_t dataCallback(
            AAudioStream *stream, void *userData, void *audioData, int32_t numFrames);

    AAudioStream *mStream;
    std::atomic<float> mVolume;
    std::atomic<bool> mMuted;

    std::unique_ptr<LockFreeRingBuffer<int16_t>> mRingBuffer;
    std::unique_ptr<AudioDecoder> mDecoder;
};
