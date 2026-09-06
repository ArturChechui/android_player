// AudioEngine.cpp
#include "AudioEngine.h"
#include <android/log.h>
#include <algorithm>

#define TAG "AudioEngine"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

AudioEngine::AudioEngine()
        : mStream(nullptr),
          mVolume(0.8f),
          mMuted(false) {
    mRingBuffer = std::make_unique<LockFreeRingBuffer<int16_t>>(192000); // ~2 sec buffer
    mDecoder = std::make_unique<AudioDecoder>(*mRingBuffer);
}

AudioEngine::~AudioEngine() {
    stop();
}

bool AudioEngine::start() {
    AAudioStreamBuilder *builder;
    AAudio_createStreamBuilder(&builder);

    AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
    AAudioStreamBuilder_setChannelCount(builder, 2); // Stereo
    AAudioStreamBuilder_setSampleRate(builder, 44100);
    AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
    AAudioStreamBuilder_setDataCallback(builder, dataCallback, this);

    aaudio_result_t result = AAudioStreamBuilder_openStream(builder, &mStream);
    AAudioStreamBuilder_delete(builder);

    if (result != AAUDIO_OK) {
        return false;
    }

    AAudioStream_requestStart(mStream);

    return true;
}

void AudioEngine::stop() {
    if (mStream) {
        AAudioStream_requestStop(mStream);
        AAudioStream_close(mStream);
        mStream = nullptr;
    }
}

void AudioEngine::setVolume(float volume) {
    mVolume.store(std::clamp(volume, 0.0f, 1.0f));
}

void AudioEngine::setMute(bool muted) {
    mMuted.store(muted);
}

void AudioEngine::switchStream(const std::string &url) {
    LOGI("Switching audio stream source to: %s", url.c_str());

    mDecoder->stop();

    auto source = std::make_unique<HttpDataSource>();
    if (source->open(url)) {
        // Pass network source into the decoder component
        mDecoder->start(std::move(source), "audio/mp4a-latm");
    }
}

aaudio_data_callback_result_t AudioEngine::dataCallback(
        AAudioStream *stream, void *userData, void *audioData, int32_t numFrames) {

    auto *engine = static_cast<AudioEngine *>(userData);
    auto *outputBuffer = static_cast<int16_t *>(audioData);

    bool isMuted = engine->mMuted.load(std::memory_order_relaxed);
    float currentGain = isMuted ? 0.0f : engine->mVolume.load(std::memory_order_relaxed);

    int32_t samplesNeeded = numFrames * 2; // Stereo
    bool success = engine->mRingBuffer->pop(outputBuffer, samplesNeeded);

    if (!success) {
        // Fill silence on underflow
        std::fill_n(outputBuffer, samplesNeeded, 0);
    } else if (currentGain != 1.0f) {
        // Apply software gain
        for (int i = 0; i < samplesNeeded; ++i) {
            outputBuffer[i] = static_cast<int16_t>(outputBuffer[i] * currentGain);
        }
    }

    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}
