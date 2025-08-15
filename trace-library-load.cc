#include <napi.h>
#include <stdio.h>
#include <dlfcn.h>

// Global constructor that runs during library load
__attribute__((constructor))
void trace_constructor() {
    fprintf(stderr, "[TRACE] Library constructor executing...\n");
    fprintf(stderr, "[TRACE] This runs BEFORE any Go functions are called\n");
}

// Forward declare Go functions but don't call them
extern "C" {
    int32_t WarmupGoRuntime();
    int32_t SetupJson(void* configJson);
    int32_t Encrypt(void* partition_id, void* data);
}

// Test function that explicitly calls Go
Napi::Value CallSetupExplicitly(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    fprintf(stderr, "[TRACE] About to call SetupJson explicitly...\n");
    
    // This should trigger the crypto init if it hasn't happened yet
    // Use a dummy config that will fail gracefully
    char dummy_config[] = "{\"invalid\":\"config\"}";
    int32_t result = SetupJson((void*)dummy_config);
    
    fprintf(stderr, "[TRACE] SetupJson returned: %d\n", result);
    
    return Napi::Number::New(env, result);
}

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    fprintf(stderr, "[TRACE] Module Init executing...\n");
    fprintf(stderr, "[TRACE] Go function addresses:\n");
    fprintf(stderr, "[TRACE]   WarmupGoRuntime: %p\n", (void*)WarmupGoRuntime);
    fprintf(stderr, "[TRACE]   SetupJson: %p\n", (void*)SetupJson);
    fprintf(stderr, "[TRACE]   Encrypt: %p\n", (void*)Encrypt);
    
    exports.Set(Napi::String::New(env, "callSetup"), 
                Napi::Function::New(env, CallSetupExplicitly));
    
    fprintf(stderr, "[TRACE] Module Init complete - no Go functions called yet\n");
    return exports;
}

NODE_API_MODULE(trace_load, Init)