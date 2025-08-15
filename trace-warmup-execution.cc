#include <napi.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// Forward declare the warmup function from libasherah
extern "C" int32_t WarmupGoRuntime();

// Signal handler to catch hangs
void hang_handler(int sig) {
    fprintf(stderr, "[HANG DETECTED] Signal %d received - WarmupGoRuntime likely hung\n", sig);
    exit(1);
}

// Library entry point with detailed tracing
__attribute__((constructor))
void library_init() {
    fprintf(stderr, "[TRACE] Library constructor called\n");
    
    // Set up hang detection
    signal(SIGALRM, hang_handler);
    alarm(10); // 10 second timeout
    
    fprintf(stderr, "[TRACE] About to call WarmupGoRuntime...\n");
    fprintf(stderr, "[TRACE] Function address: %p\n", (void*)WarmupGoRuntime);
    
    int32_t result = WarmupGoRuntime();
    
    alarm(0); // Cancel timeout
    fprintf(stderr, "[TRACE] WarmupGoRuntime returned: %d\n", result);
}

// Simple test function
Napi::String TestFunction(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::String::New(env, "Trace test successful");
}

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    fprintf(stderr, "[TRACE] Module Init called\n");
    exports.Set(Napi::String::New(env, "test"), Napi::Function::New(env, TestFunction));
    return exports;
}

NODE_API_MODULE(trace_warmup, Init)