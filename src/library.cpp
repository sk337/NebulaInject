#include <jni.h>

#include <chrono>
#include <thread>

#include "Phantom.h"

void startClient() {
    JavaVM* jvm = nullptr;
    JNIEnv* env = nullptr;

    while (true) {
        jint res = JNI_GetCreatedJavaVMs(&jvm, 1, nullptr);
        if (res == JNI_OK && jvm != nullptr) {
            if (jvm->GetEnv((void**)&env, JNI_VERSION_1_8) == JNI_OK) break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    (new Phantom())->runClient();
}

__attribute__((constructor)) void dllLoad() {
    std::thread(startClient).detach();
}
