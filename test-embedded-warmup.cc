#include <napi.h>
#include <dlfcn.h>
#include <stdio.h>

// Function pointer for the warmup
typedef int (*WarmupFunc)();

// This runs during module initialization, BEFORE any JavaScript code
static void WarmupGoRuntime() {
    fprintf(stderr, "[Embedded] Warming up Go runtime...\n");
    
    // Load the warmup library
    void* handle = dlopen("./go-runtime-warmup.dylib", RTLD_LAZY | RTLD_GLOBAL);
    if (!handle) {
        fprintf(stderr, "[Embedded] Failed to load warmup: %s\n", dlerror());
        return;
    }
    
    // Get the warmup function
    WarmupFunc warmup = (WarmupFunc)dlsym(handle, "WarmupGoRuntime");
    if (!warmup) {
        fprintf(stderr, "[Embedded] Failed to find WarmupGoRuntime: %s\n", dlerror());
        return;
    }
    
    // Call it
    int result = warmup();
    fprintf(stderr, "[Embedded] Warmup result: %d\n", result);
}

// Simple test function
Napi::Value TestFunction(const Napi::CallbackInfo& info) {
    return Napi::String::New(info.Env(), "Module with embedded warmup works!");
}

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    fprintf(stderr, "[Module] Init called\n");
    
    // Warm up Go runtime DURING module init
    WarmupGoRuntime();
    
    fprintf(stderr, "[Module] Setting up exports\n");
    exports.Set("test", Napi::Function::New(env, TestFunction));
    
    fprintf(stderr, "[Module] Init complete\n");
    return exports;
}

NODE_API_MODULE(test_embedded_warmup, Init)