#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <media/NdkMediaCodec.h>
#include "HttpDataSource.h"
#include "LockFreeRingBuffer.h"

class AudioDecoder {
public:
    explicit AudioDecoder(LockFreeRingBuffer<int16_t> &pcmTargetBuffer);

    ~AudioDecoder();

    void start(std::unique_ptr<HttpDataSource> dataSource, const char *mimeType = "audio/mpeg");

    void stop();

private:
    void decodeLoop();

    std::unique_ptr<HttpDataSource> mDataSource;
    LockFreeRingBuffer<int16_t> &mPcmBuffer;
    std::thread mDecodeThread;
    std::atomic<bool> mIsRunning{false};
    std::string mMimeType;
};