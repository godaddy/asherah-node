#include <napi.h>
#include <stdio.h>

// Test: what if we do "warmup" without calling Go at all?
// Maybe the issue is just that Bun needs some basic initialization
static int cpp_warmup_result = []() {
    fprintf(stderr, "[CPP-WARMUP] Performing C++ only warmup...\n");
    
    // Some basic operations that might help with runtime compatibility
    // without triggering Go runtime
    volatile int sum = 0;
    for (int i = 0; i < 1000; i++) {
        sum += i;
    }
    
    fprintf(stderr, "[CPP-WARMUP] C++ warmup complete, sum: %d\n", sum);
    return sum;
}();

// Load the actual asherah library normally (after warmup)
extern "C" {
    // Forward declare from libasherah
    int32_t SetupJson(void* configJson);
    int32_t Encrypt(void* partition_id, void* data);
    int32_t Shutdown();
}

// Simple test function that tries to use asherah
Napi::String TestFunction(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    fprintf(stderr, "[CPP-WARMUP] Test function called after warmup\n");
    
    // Don't actually call asherah functions - just verify we can access them
    fprintf(stderr, "[CPP-WARMUP] SetupJson function at: %p\n", (void*)SetupJson);
    fprintf(stderr, "[CPP-WARMUP] Encrypt function at: %p\n", (void*)Encrypt);
    
    return Napi::String::New(env, "C++ only warmup test successful");
}

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    fprintf(stderr, "[CPP-WARMUP] Module Init called, warmup result was: %d\n", cpp_warmup_result);
    exports.Set(Napi::String::New(env, "test"), Napi::Function::New(env, TestFunction));
    return exports;
}

NODE_API_MODULE(test_cpp_warmup, Init)