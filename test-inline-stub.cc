#include <napi.h>
#include <stdio.h>
#include <dlfcn.h>

// Test calling our minimal stub function during static initialization
static int stub_result = []() {
    fprintf(stderr, "[INLINE-STUB] Testing minimal stub approach...\n");
    
    // Load our minimal stub library
    void* handle = dlopen("./test_minimal_stub.dylib", RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "[INLINE-STUB] Failed to load stub lib: %s\n", dlerror());
        return -1;
    }
    
    typedef int (*MinimalStubFunc)();
    MinimalStubFunc stub = (MinimalStubFunc)dlsym(handle, "MinimalStub");
    if (!stub) {
        fprintf(stderr, "[INLINE-STUB] Failed to find MinimalStub: %s\n", dlerror());
        dlclose(handle);
        return -2;
    }
    
    fprintf(stderr, "[INLINE-STUB] Calling MinimalStub...\n");
    int result = stub();
    fprintf(stderr, "[INLINE-STUB] MinimalStub returned: %d\n", result);
    
    dlclose(handle);
    return result;
}();

// Simple test function
Napi::String TestFunction(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::String::New(env, "Inline stub test successful");
}

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    fprintf(stderr, "[INLINE-STUB] Module Init called, stub_result was: %d\n", stub_result);
    exports.Set(Napi::String::New(env, "test"), Napi::Function::New(env, TestFunction));
    return exports;
}

NODE_API_MODULE(test_inline_stub, Init)