#include <napi.h>
#include <stdio.h>
#include <pthread.h>
#include <dlfcn.h>
#include <signal.h>
#include <string.h>

extern "C" {
    // Go runtime initialization functions
    void _rt0_arm64_darwin_lib();
    void _rt0_arm64_darwin_lib_go();
    void x_cgo_init(void* g, void* setg);
    void x_cgo_notify_runtime_init_done();
    int _cgo_wait_runtime_init_done();
    
    // The actual function
    int SetupJson(void* buffer);
}

Napi::Value TestDirectInit(const Napi::CallbackInfo& info) {
    fprintf(stderr, "\n=== Attempting Direct Go Runtime Init ===\n");
    
    // Try 1: Call runtime init directly
    fprintf(stderr, "1. Calling _cgo_wait_runtime_init_done...\n");
    fflush(stderr);
    
    int init_result = _cgo_wait_runtime_init_done();
    fprintf(stderr, "   Result: %d\n", init_result);
    
    // Now try SetupJson
    fprintf(stderr, "\n2. Calling SetupJson after init wait...\n");
    char buffer[16] = {0};
    *(int32_t*)buffer = 2;
    strcpy(buffer + 8, "{}");
    
    int result = SetupJson(buffer);
    fprintf(stderr, "   SetupJson returned: %d\n", result);
    
    return Napi::Number::New(info.Env(), result);
}

// Try initializing in a clean thread
void* init_thread(void* arg) {
    fprintf(stderr, "\n=== Init Thread ===\n");
    
    // Clear all signals
    sigset_t set;
    sigemptyset(&set);
    pthread_sigmask(SIG_SETMASK, &set, NULL);
    
    fprintf(stderr, "Calling _rt0_arm64_darwin_lib...\n");
    fflush(stderr);
    
    _rt0_arm64_darwin_lib();
    
    fprintf(stderr, "Runtime lib initialized\n");
    return NULL;
}

Napi::Value TestThreadInit(const Napi::CallbackInfo& info) {
    fprintf(stderr, "\n=== Testing Thread-based Init ===\n");
    
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    
    // Set large stack
    size_t stacksize = 8 * 1024 * 1024;
    pthread_attr_setstacksize(&attr, stacksize);
    
    fprintf(stderr, "Creating init thread with 8MB stack...\n");
    
    if (pthread_create(&thread, &attr, init_thread, NULL) != 0) {
        fprintf(stderr, "Failed to create thread\n");
        return Napi::Number::New(info.Env(), -1);
    }
    
    pthread_join(thread, NULL);
    pthread_attr_destroy(&attr);
    
    fprintf(stderr, "Init thread completed\n");
    
    // Now try SetupJson
    fprintf(stderr, "Attempting SetupJson...\n");
    char buffer[16] = {0};
    *(int32_t*)buffer = 2;
    strcpy(buffer + 8, "{}");
    
    int result = SetupJson(buffer);
    fprintf(stderr, "SetupJson returned: %d\n", result);
    
    return Napi::Number::New(info.Env(), result);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("directInit", Napi::Function::New(env, TestDirectInit));
    exports.Set("threadInit", Napi::Function::New(env, TestThreadInit));
    return exports;
}

NODE_API_MODULE(go_init_direct, Init)