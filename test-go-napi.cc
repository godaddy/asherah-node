#include <napi.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <dlfcn.h>

// Go interop types
typedef unsigned char GoUint8;

// SetupJson expects a pointer to a Cobhan buffer
// Buffer format: [4-byte length][data...]
extern "C" int SetupJson(GoUint8* buffer);

void print_thread_info(const char* context) {
    pthread_t tid = pthread_self();
    void* stack_addr = pthread_get_stackaddr_np(tid);
    size_t stack_size = pthread_get_stacksize_np(tid);
    
    sigset_t mask;
    pthread_sigmask(SIG_BLOCK, NULL, &mask);
    
    fprintf(stderr, "\n=== %s ===\n", context);
    fprintf(stderr, "Thread: %p, Stack: %zu MB, Main: %s\n", 
            (void*)tid, stack_size / (1024*1024), 
            pthread_main_np() ? "YES" : "NO");
    fprintf(stderr, "Blocked signals:");
    for (int i = 1; i < 32; i++) {
        if (sigismember(&mask, i)) fprintf(stderr, " %d", i);
    }
    fprintf(stderr, "\n");
}

// Create Cobhan buffer with proper format
GoUint8* create_cobhan_buffer(const char* str) {
    size_t len = strlen(str);
    GoUint8* buffer = (GoUint8*)malloc(len + 4);
    
    // Write 4-byte length header (little-endian)
    int32_t length = (int32_t)len;
    memcpy(buffer, &length, 4);
    
    // Copy string data
    memcpy(buffer + 4, str, len);
    
    return buffer;
}

// Test calling Go directly from main thread
Napi::Value TestGoSync(const Napi::CallbackInfo& info) {
    print_thread_info("N-API Sync (before Go call)");
    
    const char* config = "{\"ServiceName\":\"test\",\"ProductID\":\"test\",\"KMS\":\"static\",\"Metastore\":\"memory\"}";
    GoUint8* buffer = create_cobhan_buffer(config);
    
    fprintf(stderr, "Calling SetupJson...\n");
    int result = SetupJson(buffer);
    fprintf(stderr, "SetupJson returned: %d\n", result);
    
    free(buffer);
    return Napi::Number::New(info.Env(), result);
}

// Async worker for Go call
class GoAsyncWorker : public Napi::AsyncWorker {
public:
    GoAsyncWorker(Napi::Env env) : Napi::AsyncWorker(env), deferred(env) {}
    
    Napi::Promise Promise() { return deferred.Promise(); }
    
protected:
    void Execute() override {
        print_thread_info("N-API Async (in Execute, before Go call)");
        
        const char* config = "{\"ServiceName\":\"test\",\"ProductID\":\"test\",\"KMS\":\"static\",\"Metastore\":\"memory\"}";
        GoUint8* buffer = create_cobhan_buffer(config);
        
        fprintf(stderr, "Calling SetupJson from async worker...\n");
        result = SetupJson(buffer);
        fprintf(stderr, "SetupJson returned: %d\n", result);
        
        free(buffer);
    }
    
    void OnOK() override {
        deferred.Resolve(Napi::Number::New(Env(), result));
    }
    
private:
    Napi::Promise::Deferred deferred;
    int result;
};

Napi::Value TestGoAsync(const Napi::CallbackInfo& info) {
    print_thread_info("N-API Async (before queue)");
    auto worker = new GoAsyncWorker(info.Env());
    worker->Queue();
    return worker->Promise();
}

// FFI-compatible direct call
extern "C" void test_go_ffi() {
    print_thread_info("FFI Direct (before Go call)");
    
    const char* config = "{\"ServiceName\":\"test\",\"ProductID\":\"test\",\"KMS\":\"static\",\"Metastore\":\"memory\"}";
    GoUint8* buffer = create_cobhan_buffer(config);
    
    fprintf(stderr, "Calling SetupJson from FFI...\n");
    int result = SetupJson(buffer);
    fprintf(stderr, "SetupJson returned: %d\n", result);
    
    free(buffer);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("testGoSync", Napi::Function::New(env, TestGoSync));
    exports.Set("testGoAsync", Napi::Function::New(env, TestGoAsync));
    return exports;
}

NODE_API_MODULE(go_napi, Init)