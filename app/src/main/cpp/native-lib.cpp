#include <jni.h>
#include <string>
#include <aaudio/AAudio.h>
#include <cmath>

#define TWO_PI 6.283185307179586

AAudioStream *audioStream = nullptr;
double phase = 0.0;
double phaseIncrement = 440.0 * TWO_PI / 48000.0; // 440Hz Sine wave

// Audio callback function: runs on a high-priority real-time audio thread
aaudio_data_callback_result_t audioCallback(
        AAudioStream *stream,
        void *userData,
        void *audioData,
        int32_t numFrames) {

    auto buffer = static_cast<int16_t*>(audioData);
    for (int i = 0; i < numFrames; i++) {
        buffer[i] = (int16_t)(sin(phase) * 32767.0); // Generate 16-bit PCM wave
        phase += phaseIncrement;
        if (phase >= TWO_PI) phase -= TWO_PI;
    }
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_player_MainActivity_startAudio(JNIEnv* env, jobject /* this */) {
    AAudioStreamBuilder *builder;
    AAudio_createStreamBuilder(&builder);

    AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
    AAudioStreamBuilder_setChannelCount(builder, 1);
    AAudioStreamBuilder_setSampleRate(builder, 48000);
    AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
    AAudioStreamBuilder_setDataCallback(builder, audioCallback, nullptr);

    AAudioStreamBuilder_openStream(builder, &audioStream);
    AAudioStream_requestStart(audioStream);
    AAudioStreamBuilder_delete(builder);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_player_MainActivity_stopAudio(JNIEnv* env, jobject /* this */) {
    if (audioStream != nullptr) {
        AAudioStream_requestStop(audioStream);
        AAudioStream_close(audioStream);
        audioStream = nullptr;
    }
}
