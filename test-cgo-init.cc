#include <napi.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <setjmp.h>

extern "C" {
    // CGO initialization functions
    void x_cgo_init(void* g, void* setg);
    void* x_cgo_thread_start(void* arg);
    void crosscall1(void (*fn)(void*), void* arg);
    void crosscall2(void (*fn)(void*, int, size_t*), void* arg, int n, size_t* stk);
    
    // Go functions
    int SetupJson(void* buffer);
    void _cgo_wait_runtime_init_done();
    
    // Thread-local storage
    void* _cgo_get_context();
    void _cgo_set_context(void* ctx);
}

// CGO expects these thread-local storage functions
static __thread void* g_context = nullptr;

extern "C" void* _cgo_get_context() {
    return g_context;
}

extern "C" void _cgo_set_context(void* ctx) {
    g_context = ctx;
}

// Dummy setg function for x_cgo_init
static void dummy_setg(void* g) {
    fprintf(stderr, "dummy_setg called with g=%p\n", g);
    _cgo_set_context(g);
}

Napi::Value TestCgoInit(const Napi::CallbackInfo& info) {
    fprintf(stderr, "\n=== Testing CGO Initialization ===\n");
    
    // Step 1: Initialize CGO
    fprintf(stderr, "1. Calling x_cgo_init...\n");
    x_cgo_init(nullptr, (void*)dummy_setg);
    fprintf(stderr, "   x_cgo_init completed\n");
    
    // Step 2: Wait for runtime
    fprintf(stderr, "\n2. Waiting for runtime init...\n");
    _cgo_wait_runtime_init_done();
    fprintf(stderr, "   Runtime initialized\n");
    
    // Step 3: Call SetupJson
    fprintf(stderr, "\n3. Calling SetupJson...\n");
    char buffer[16] = {0};
    *(int32_t*)buffer = 2;
    strcpy(buffer + 8, "{}");
    
    int result = SetupJson(buffer);
    fprintf(stderr, "   SetupJson returned: %d\n", result);
    
    return Napi::Number::New(info.Env(), result);
}

// Thread function that properly initializes CGO
void* cgo_thread_func(void* arg) {
    fprintf(stderr, "\n=== CGO Thread ===\n");
    
    // Initialize thread for CGO
    fprintf(stderr, "Calling x_cgo_thread_start...\n");
    x_cgo_thread_start(arg);
    
    fprintf(stderr, "Thread initialized for CGO\n");
    
    // Now try SetupJson
    char buffer[16] = {0};
    *(int32_t*)buffer = 2;
    strcpy(buffer + 8, "{}");
    
    fprintf(stderr, "Calling SetupJson from CGO thread...\n");
    int result = SetupJson(buffer);
    fprintf(stderr, "SetupJson returned: %d\n", result);
    
    return (void*)(intptr_t)result;
}

Napi::Value TestCgoThread(const Napi::CallbackInfo& info) {
    fprintf(stderr, "\n=== Testing CGO Thread Start ===\n");
    
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 8 * 1024 * 1024);
    
    fprintf(stderr, "Creating CGO-aware thread...\n");
    
    if (pthread_create(&thread, &attr, cgo_thread_func, nullptr) != 0) {
        fprintf(stderr, "Failed to create thread\n");
        return Napi::Number::New(info.Env(), -1);
    }
    
    void* result;
    pthread_join(thread, &result);
    pthread_attr_destroy(&attr);
    
    fprintf(stderr, "Thread completed with result: %d\n", (int)(intptr_t)result);
    
    return Napi::Number::New(info.Env(), (int)(intptr_t)result);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("initCgo", Napi::Function::New(env, TestCgoInit));
    exports.Set("threadCgo", Napi::Function::New(env, TestCgoThread));
    return exports;
}

NODE_API_MODULE(cgo_init, Init)