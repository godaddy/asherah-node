#include <napi.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Go types
typedef int32_t GoInt32;

// The actual SetupJson function we want to call
extern "C" GoInt32 SetupJson(void* config);

// Go warmup function is provided by the Go library
extern "C" GoInt32 WarmupGoRuntime();

void print_thread_info(const char* context) {
    pthread_t tid = pthread_self();
    fprintf(stderr, "%s - Thread: %p\n", context, (void*)tid);
}

// Test sync call
Napi::Value TestSetupSyncWithWarmup(const Napi::CallbackInfo& info) {
    print_thread_info("TestSetupSyncWithWarmup");
    
    const char* config = "{\"ServiceName\":\"test\",\"ProductID\":\"test\",\"KMS\":\"static\",\"Metastore\":\"memory\"}";
    size_t len = strlen(config);
    size_t alloc_size = len + 8;
    char* buffer = (char*)calloc(1, alloc_size);
    
    int32_t length = (int32_t)len;
    memcpy(buffer, &length, 4);
    memcpy(buffer + 8, config, len);
    
    fprintf(stderr, "Calling SetupJson...\n");
    GoInt32 result = SetupJson(buffer);
    fprintf(stderr, "SetupJson returned: %d\n", result);
    
    free(buffer);
    return Napi::Number::New(info.Env(), result);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    fprintf(stderr, "[Self-Warmup] Module Init starting\n");
    
    // CRITICAL: Warm up Go runtime BEFORE exporting any functions
    fprintf(stderr, "[Self-Warmup] Calling WarmupGoRuntime...\n");
    GoInt32 warmupResult = WarmupGoRuntime();
    fprintf(stderr, "[Self-Warmup] WarmupGoRuntime returned: %d\n", warmupResult);
    
    // Now export the actual functions
    exports.Set("setupSync", Napi::Function::New(env, TestSetupSyncWithWarmup));
    
    fprintf(stderr, "[Self-Warmup] Module Init complete\n");
    return exports;
}

NODE_API_MODULE(test_self_warmup, Init)