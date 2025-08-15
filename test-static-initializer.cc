#include <napi.h>
#include <stdio.h>

// Forward declare the warmup function from libasherah
extern "C" int32_t WarmupGoRuntime();

// Static initializer - runs during library load
static int warmup_result = []() {
    fprintf(stderr, "[Static Init] Calling WarmupGoRuntime...\n");
    int32_t result = WarmupGoRuntime();
    fprintf(stderr, "[Static Init] WarmupGoRuntime returned: %d\n", result);
    return result;
}();

// Simple test function
Napi::String TestFunction(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::String::New(env, "Static initializer test successful");
}

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    fprintf(stderr, "[Module Init] Entering module initialization...\n");
    exports.Set(Napi::String::New(env, "test"), Napi::Function::New(env, TestFunction));
    fprintf(stderr, "[Module Init] Module initialization complete\n");
    return exports;
}

NODE_API_MODULE(test_static_init, Init)