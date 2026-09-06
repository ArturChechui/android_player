#define DR_MP3_IMPLEMENTATION

#include "dr_mp3.h"
#include "AudioDecoder.h"
#include <android/log.h>

#define LOG_TAG "AudioDecoder"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

AudioDecoder::AudioDecoder(LockFreeRingBuffer<int16_t> &pcmTargetBuffer)
        : mPcmBuffer(pcmTargetBuffer) {}

AudioDecoder::~AudioDecoder() {
    stop();
}

void AudioDecoder::start(std::unique_ptr<HttpDataSource> dataSource, const char *mimeType) {
    stop();
    mDataSource = std::move(dataSource);
    mIsRunning = true;
    mDecodeThread = std::thread(&AudioDecoder::decodeLoop, this);
}

void AudioDecoder::stop() {
    mIsRunning = false;
    if (mDecodeThread.joinable()) {
        mDecodeThread.join();
    }
    if (mDataSource) {
        mDataSource->close();
    }
}

// Custom read callback for dr_mp3 from socket
static size_t onReadProc(void *pUserData, void *pBufferOut, size_t bytesToRead) {
    auto *source = static_cast<HttpDataSource *>(pUserData);
    ssize_t readBytes = source->read(static_cast<uint8_t *>(pBufferOut), bytesToRead);
    return readBytes > 0 ? static_cast<size_t>(readBytes) : 0;
}

void AudioDecoder::decodeLoop() {
    drmp3 mp3;

    // Correct signature: 7 arguments required by dr_mp3
    drmp3_bool32 success = drmp3_init(
            &mp3,            // 1. Pointer to drmp3 struct
            onReadProc,      // 2. Read callback
            nullptr,         // 3. Seek callback (nullptr for network sockets)
            nullptr,         // 4. Tell callback (nullptr for network sockets)
            nullptr,         // 5. Meta callback (nullptr)
            mDataSource.get(), // 6. Custom user data passed into onReadProc
            nullptr          // 7. Allocation callbacks (nullptr for default malloc)
    );

    if (!success) {
        LOGI("Failed to initialize MP3 decoder stream");
        return;
    }

    LOGI("MP3 Stream initialized: %d Hz, %d channels", mp3.sampleRate, mp3.channels);

    int16_t pcmBuffer[2048];
    while (mIsRunning) {
        drmp3_uint64 framesRead = drmp3_read_pcm_frames_s16(&mp3, 1024, pcmBuffer);
        if (framesRead == 0) break;

        size_t samplesRead = framesRead * mp3.channels;
        while (mIsRunning && !mPcmBuffer.push(pcmBuffer, samplesRead)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    drmp3_uninit(&mp3);
    LOGI("Audio decode thread stopped cleanly.");
}