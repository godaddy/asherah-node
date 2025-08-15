#include <napi.h>
#include <dlfcn.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

// Test progressive Go runtime initialization
extern "C" {
    // Go runtime functions we can try to call directly
    void runtime_init();
    void runtime_main();
    void runtime_goexit();
    int SetupJson(void* buffer);
    
    // Check if runtime is initialized
    void* runtime_g();
}

Napi::Value TestGoRuntime(const Napi::CallbackInfo& info) {
    fprintf(stderr, "\n=== Testing Go Runtime Initialization ===\n");
    
    // Test 1: Check if we can access runtime symbols
    fprintf(stderr, "1. Checking runtime symbols...\n");
    void* handle = dlopen(nullptr, RTLD_LAZY | RTLD_GLOBAL);
    
    void* runtime_init_sym = dlsym(handle, "runtime_init");
    void* runtime_main_sym = dlsym(handle, "runtime_main");
    void* setup_json_sym = dlsym(handle, "SetupJson");
    
    fprintf(stderr, "   runtime_init: %p\n", runtime_init_sym);
    fprintf(stderr, "   runtime_main: %p\n", runtime_main_sym);
    fprintf(stderr, "   SetupJson: %p\n", setup_json_sym);
    
    // Test 2: Check thread state
    fprintf(stderr, "\n2. Current thread state:\n");
    pthread_t tid = pthread_self();
    size_t stack_size = pthread_get_stacksize_np(tid);
    fprintf(stderr, "   Thread ID: %p\n", (void*)tid);
    fprintf(stderr, "   Stack size: %zu MB\n", stack_size / (1024*1024));
    fprintf(stderr, "   Main thread: %s\n", pthread_main_np() ? "YES" : "NO");
    
    // Test 3: Check signal mask
    fprintf(stderr, "\n3. Signal mask:\n");
    sigset_t mask;
    pthread_sigmask(SIG_BLOCK, NULL, &mask);
    fprintf(stderr, "   Blocked signals:");
    for (int i = 1; i < 32; i++) {
        if (sigismember(&mask, i)) fprintf(stderr, " %d", i);
    }
    fprintf(stderr, "\n");
    
    // Test 4: Try calling a simple Go function that doesn't init runtime
    fprintf(stderr, "\n4. Testing minimal Go call...\n");
    
    // Create minimal buffer
    char buffer[16] = {0};
    *(int32_t*)buffer = 8; // Length = 8
    strcpy(buffer + 8, "{}");
    
    fprintf(stderr, "   About to call SetupJson...\n");
    fprintf(stderr, "   [This is where it hangs]\n");
    fflush(stderr);
    
    // This should hang
    int result = SetupJson(buffer);
    fprintf(stderr, "   SetupJson returned: %d\n", result);
    
    return Napi::Number::New(info.Env(), result);
}

// Test with pre-initialized thread
class PreInitWorker : public Napi::AsyncWorker {
public:
    PreInitWorker(Napi::Env env) : Napi::AsyncWorker(env), deferred(env) {}
    
    Napi::Promise Promise() { return deferred.Promise(); }
    
protected:
    void Execute() override {
        fprintf(stderr, "\n=== Async Worker Thread ===\n");
        
        // Set up thread environment similar to Go's expectations
        sigset_t set;
        sigemptyset(&set);
        pthread_sigmask(SIG_SETMASK, &set, NULL);
        
        // Try to init Go runtime
        char buffer[16] = {0};
        *(int32_t*)buffer = 8;
        strcpy(buffer + 8, "{}");
        
        fprintf(stderr, "Calling SetupJson from worker...\n");
        fflush(stderr);
        
        result = SetupJson(buffer);
        fprintf(stderr, "Worker: SetupJson returned: %d\n", result);
    }
    
    void OnOK() override {
        deferred.Resolve(Napi::Number::New(Env(), result));
    }
    
private:
    Napi::Promise::Deferred deferred;
    int result;
};

Napi::Value TestGoAsync(const Napi::CallbackInfo& info) {
    auto worker = new PreInitWorker(info.Env());
    worker->Queue();
    return worker->Promise();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("testRuntime", Napi::Function::New(env, TestGoRuntime));
    exports.Set("testAsync", Napi::Function::New(env, TestGoAsync));
    return exports;
}

NODE_API_MODULE(go_trace, Init)