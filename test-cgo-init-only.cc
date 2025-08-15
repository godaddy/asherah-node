#include <napi.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// Try lower-level CGO initialization functions
extern "C" {
    void _x_cgo_init();
    void __cgo_wait_runtime_init_done();
    void _x_cgo_notify_runtime_init_done();
    void _x_cgo_bindm();
}

// Signal handler for hang detection
void hang_handler(int sig) {
    fprintf(stderr, "[CGO-INIT] Signal %d - function hung\n", sig);
    exit(1);
}

// Test calling lower-level CGO init functions
static int cgo_init_result = []() {
    fprintf(stderr, "[CGO-INIT] Testing lower-level CGO initialization...\n");
    
    signal(SIGALRM, hang_handler);
    alarm(5);
    
    try {
        // Try the most basic CGO init
        fprintf(stderr, "[CGO-INIT] Calling _x_cgo_init()...\n");
        _x_cgo_init();
        fprintf(stderr, "[CGO-INIT] ✅ _x_cgo_init() completed\n");
        
        // Try bindm (bind to OS thread)
        fprintf(stderr, "[CGO-INIT] Calling _x_cgo_bindm()...\n");
        _x_cgo_bindm();
        fprintf(stderr, "[CGO-INIT] ✅ _x_cgo_bindm() completed\n");
        
        // Try waiting for runtime init
        fprintf(stderr, "[CGO-INIT] Calling __cgo_wait_runtime_init_done()...\n");
        __cgo_wait_runtime_init_done();
        fprintf(stderr, "[CGO-INIT] ✅ __cgo_wait_runtime_init_done() completed\n");
        
        alarm(0);
        return 1;
    } catch (...) {
        alarm(0);
        fprintf(stderr, "[CGO-INIT] Exception during CGO init\n");
        return -1;
    }
}();

// Simple test function
Napi::String TestFunction(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::String::New(env, "CGO init test successful");
}

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    fprintf(stderr, "[CGO-INIT] Module Init called, cgo_init_result: %d\n", cgo_init_result);
    exports.Set(Napi::String::New(env, "test"), Napi::Function::New(env, TestFunction));
    return exports;
}

NODE_API_MODULE(test_cgo_init, Init)