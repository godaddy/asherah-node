#include <napi.h>
#include <stdio.h>
#include <pthread.h>
#include <dlfcn.h>

extern "C" {
    // CGO functions
    int SetupJson(void* buffer);
    
    // Our shim functions
    void x_cgo_init(void* g, void* setg);
    void _cgo_wait_runtime_init_done();
    void* x_cgo_thread_start(void* arg);
    void* _cgo_get_context();
    void _cgo_set_context(void* ctx);
}

// Dummy setg function
static void bun_setg(void* g) {
    fprintf(stderr, "bun_setg: setting g=%p\n", g);
    _cgo_set_context(g);
}

Napi::Value TestShimInit(const Napi::CallbackInfo& info) {
    fprintf(stderr, "\n=== Testing CGO Shim ===\n");
    
    // Initialize CGO with our shim
    fprintf(stderr, "1. Initializing CGO shim...\n");
    x_cgo_init((void*)0x1000, (void*)bun_setg);
    
    // Wait for init
    fprintf(stderr, "2. Waiting for runtime...\n");
    _cgo_wait_runtime_init_done();
    
    // Call SetupJson
    fprintf(stderr, "3. Calling SetupJson...\n");
    char buffer[16] = {0};
    *(int32_t*)buffer = 2;
    strcpy(buffer + 8, "{}");
    
    int result = SetupJson(buffer);
    fprintf(stderr, "SetupJson returned: %d\n", result);
    
    return Napi::Number::New(info.Env(), result);
}

// Async worker with CGO context
class CgoAsyncWorker : public Napi::AsyncWorker {
public:
    CgoAsyncWorker(Napi::Env env) : Napi::AsyncWorker(env), deferred(env) {}
    
    Napi::Promise Promise() { return deferred.Promise(); }
    
protected:
    void Execute() override {
        fprintf(stderr, "\n=== Async Worker with CGO ===\n");
        
        // Initialize this thread for CGO
        fprintf(stderr, "Initializing thread for CGO...\n");
        x_cgo_thread_start(nullptr);
        
        // Call SetupJson
        char buffer[16] = {0};
        *(int32_t*)buffer = 2;
        strcpy(buffer + 8, "{}");
        
        fprintf(stderr, "Calling SetupJson from async worker...\n");
        result = SetupJson(buffer);
        fprintf(stderr, "Async worker: SetupJson returned: %d\n", result);
    }
    
    void OnOK() override {
        deferred.Resolve(Napi::Number::New(Env(), result));
    }
    
private:
    Napi::Promise::Deferred deferred;
    int result;
};

Napi::Value TestShimAsync(const Napi::CallbackInfo& info) {
    auto worker = new CgoAsyncWorker(info.Env());
    worker->Queue();
    return worker->Promise();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("shimInit", Napi::Function::New(env, TestShimInit));
    exports.Set("shimAsync", Napi::Function::New(env, TestShimAsync));
    return exports;
}

NODE_API_MODULE(cgo_shim, Init)