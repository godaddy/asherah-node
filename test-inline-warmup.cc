#include <napi.h>
#include <stdio.h>

// Static initializer with inline minimal warmup (no Go calls)
static int warmup_result = []() {
    fprintf(stderr, "[Inline Warmup] Performing minimal JavaScript engine warmup...\n");
    // Just do some basic initialization that doesn't involve Go/CGO
    return 1;
}();

// Simple test function
Napi::String TestFunction(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::String::New(env, "Inline warmup test successful");
}

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    fprintf(stderr, "[Module Init] Entering module initialization...\n");
    exports.Set(Napi::String::New(env, "test"), Napi::Function::New(env, TestFunction));
    fprintf(stderr, "[Module Init] Module initialization complete\n");
    return exports;
}

NODE_API_MODULE(test_inline_warmup, Init)