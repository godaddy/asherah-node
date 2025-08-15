#include <napi.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// Check if there's already a simple function we can use from libasherah
// Let's try EstimateBuffer which should be simpler than WarmupGoRuntime
extern "C" int32_t EstimateBuffer(void* partition_id);

// Or if there's a SetEnv function that might be simpler
extern "C" int32_t SetEnv(void* env_json);

// Signal handler for hang detection
void hang_handler(int sig) {
    fprintf(stderr, "[HANG] Signal %d - function appears to have hung\n", sig);
    exit(1);
}

// Test calling a simpler Go function during static initialization
static int stub_result = []() {
    fprintf(stderr, "[STUB] Testing simpler Go function calls...\n");
    
    // Set up hang detection
    signal(SIGALRM, hang_handler);
    alarm(5);
    
    try {
        // Try EstimateBuffer with null (should be safe)
        fprintf(stderr, "[STUB] Calling EstimateBuffer(nullptr)...\n");
        int32_t result = EstimateBuffer(nullptr);
        fprintf(stderr, "[STUB] EstimateBuffer returned: %d\n", result);
        
        alarm(0);
        return result;
    } catch (...) {
        alarm(0);
        fprintf(stderr, "[STUB] EstimateBuffer threw exception\n");
        return -1;
    }
}();

// Simple test function
Napi::String TestFunction(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::String::New(env, "Stub function test successful");
}

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    fprintf(stderr, "[STUB] Module Init called, stub_result was: %d\n", stub_result);
    exports.Set(Napi::String::New(env, "test"), Napi::Function::New(env, TestFunction));
    return exports;
}

NODE_API_MODULE(test_stub, Init)