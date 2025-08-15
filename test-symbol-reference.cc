#include <napi.h>
#include <stdio.h>
#include <dlfcn.h>

// Forward declare but don't call Go functions
extern "C" int32_t WarmupGoRuntime();
extern "C" int32_t SetupJson(void* configJson);

// Test just referencing Go symbols without calling them
static int symbol_ref_result = []() {
    fprintf(stderr, "[SYMBOL-REF] Testing symbol reference approach...\n");
    
    // Just get the addresses without calling
    void* warmup_addr = (void*)WarmupGoRuntime;
    void* setup_addr = (void*)SetupJson;
    
    fprintf(stderr, "[SYMBOL-REF] WarmupGoRuntime at: %p\n", warmup_addr);
    fprintf(stderr, "[SYMBOL-REF] SetupJson at: %p\n", setup_addr);
    
    // Try to use dladdr to get info about the symbols
    Dl_info warmup_info, setup_info;
    if (dladdr(warmup_addr, &warmup_info)) {
        fprintf(stderr, "[SYMBOL-REF] WarmupGoRuntime in: %s\n", warmup_info.dli_fname);
    }
    
    if (dladdr(setup_addr, &setup_info)) {
        fprintf(stderr, "[SYMBOL-REF] SetupJson in: %s\n", setup_info.dli_fname);
    }
    
    // Maybe just touching the symbols is enough to trigger some basic loading?
    fprintf(stderr, "[SYMBOL-REF] Symbol reference complete\n");
    return 1;
}();

// Simple test function
Napi::String TestFunction(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::String::New(env, "Symbol reference test successful");
}

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    fprintf(stderr, "[SYMBOL-REF] Module Init called, symbol_ref_result: %d\n", symbol_ref_result);
    exports.Set(Napi::String::New(env, "test"), Napi::Function::New(env, TestFunction));
    return exports;
}

NODE_API_MODULE(test_symbol_ref, Init)