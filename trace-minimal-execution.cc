#include <napi.h>
#include <stdio.h>
#include <dlfcn.h>

// Library entry point with detailed tracing
__attribute__((constructor))
void library_init() {
    fprintf(stderr, "[TRACE-MINIMAL] Library constructor called\n");
    
    // Try to load and call our minimal warmup
    void* handle = dlopen("./asherah-bun-preload/lib/bun_warmup_minimal.dylib", RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "[TRACE-MINIMAL] Failed to load minimal lib: %s\n", dlerror());
        return;
    }
    
    typedef int (*MinimalWarmupFunc)();
    MinimalWarmupFunc warmup = (MinimalWarmupFunc)dlsym(handle, "MinimalWarmup");
    if (!warmup) {
        fprintf(stderr, "[TRACE-MINIMAL] Failed to find MinimalWarmup: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    fprintf(stderr, "[TRACE-MINIMAL] About to call MinimalWarmup...\n");
    fprintf(stderr, "[TRACE-MINIMAL] Function address: %p\n", (void*)warmup);
    
    int result = warmup();
    
    fprintf(stderr, "[TRACE-MINIMAL] MinimalWarmup returned: %d\n", result);
    dlclose(handle);
}

// Simple test function
Napi::String TestFunction(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::String::New(env, "Trace minimal test successful");
}

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    fprintf(stderr, "[TRACE-MINIMAL] Module Init called\n");
    exports.Set(Napi::String::New(env, "test"), Napi::Function::New(env, TestFunction));
    return exports;
}

NODE_API_MODULE(trace_minimal, Init)