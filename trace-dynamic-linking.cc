#include <napi.h>
#include <stdio.h>
#include <dlfcn.h>

// Forward declare the warmup function from libasherah
extern "C" int32_t WarmupGoRuntime();

// Library entry point with detailed tracing
__attribute__((constructor))
void library_init() {
    fprintf(stderr, "[DYNAMIC] Library constructor called\n");
    
    // Check what we're actually calling
    void* warmup_addr = dlsym(RTLD_DEFAULT, "WarmupGoRuntime");
    fprintf(stderr, "[DYNAMIC] dlsym found WarmupGoRuntime at: %p\n", warmup_addr);
    
    // Check the static symbol
    fprintf(stderr, "[DYNAMIC] Static symbol WarmupGoRuntime at: %p\n", (void*)WarmupGoRuntime);
    
    // Are they the same?
    if (warmup_addr == (void*)WarmupGoRuntime) {
        fprintf(stderr, "[DYNAMIC] Addresses match - same function\n");
    } else {
        fprintf(stderr, "[DYNAMIC] Addresses differ - different functions!\n");
    }
    
    // Try to see what libraries are loaded
    Dl_info info;
    if (dladdr((void*)WarmupGoRuntime, &info)) {
        fprintf(stderr, "[DYNAMIC] WarmupGoRuntime is in: %s\n", info.dli_fname);
        fprintf(stderr, "[DYNAMIC] Base address: %p\n", info.dli_fbase);
    }
}

// Simple test function  
Napi::String TestFunction(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::String::New(env, "Dynamic linking test successful");
}

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    fprintf(stderr, "[DYNAMIC] Module Init called\n");
    exports.Set(Napi::String::New(env, "test"), Napi::Function::New(env, TestFunction));
    return exports;
}

NODE_API_MODULE(trace_dynamic, Init)