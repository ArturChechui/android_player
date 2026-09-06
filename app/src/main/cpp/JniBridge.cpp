#include <jni.h>
#include "AudioEngine.h"

static AudioEngine *gAudioEngine = nullptr;

extern "C" {

JNIEXPORT void JNICALL
Java_com_example_player_MainActivity_nativeInit(JNIEnv *env, jobject) {
    if (!gAudioEngine) {
        gAudioEngine = new AudioEngine();
        gAudioEngine->start();
    }
}

JNIEXPORT void JNICALL
Java_com_example_player_MainActivity_nativeSetVolume(JNIEnv *env, jobject, jfloat vol) {
    if (gAudioEngine) {
        gAudioEngine->setVolume(vol);
    }
}

JNIEXPORT void JNICALL
Java_com_example_player_MainActivity_nativeSetMute(JNIEnv *env, jobject, jboolean muted) {
    if (gAudioEngine) {
        gAudioEngine->setMute(muted);
    }
}

JNIEXPORT void JNICALL
Java_com_example_player_MainActivity_nativeSwitchStream(JNIEnv *env, jobject, jstring url) {
    if (gAudioEngine) {
        const char *nativeUrl = env->GetStringUTFChars(url, nullptr);
        gAudioEngine->switchStream(nativeUrl);
        env->ReleaseStringUTFChars(url, nativeUrl);
    }
}

JNIEXPORT void JNICALL
Java_com_example_player_MainActivity_nativeDestroy(JNIEnv *env, jobject) {
    if (gAudioEngine) {
        delete gAudioEngine;
        gAudioEngine = nullptr;
    }
}

}
