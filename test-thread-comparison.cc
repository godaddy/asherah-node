#include <napi.h>
#include <pthread.h>
#include <signal.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>

// Thread diagnostics function
void print_thread_info(const char* context) {
    pthread_t tid = pthread_self();
    
    // Get stack info (macOS specific)
    void* stack_addr = pthread_get_stackaddr_np(tid);
    size_t stack_size = pthread_get_stacksize_np(tid);
    
    // Get signal mask
    sigset_t mask;
    pthread_sigmask(SIG_BLOCK, NULL, &mask);
    
    printf("\n=== Thread Info: %s ===\n", context);
    printf("Thread ID: %p\n", (void*)tid);
    printf("Stack address: %p\n", stack_addr);
    printf("Stack size: %zu bytes (%zu MB)\n", stack_size, stack_size / (1024 * 1024));
    printf("Main thread: %s\n", pthread_main_np() ? "YES" : "NO");
    
    printf("Blocked signals: ");
    for (int i = 1; i < 32; i++) {
        if (sigismember(&mask, i)) {
            printf("%d ", i);
        }
    }
    printf("\n");
}

// Simple function to test
extern "C" int test_function() {
    print_thread_info("Inside test_function");
    return 42;
}

// N-API async worker
class TestAsyncWorker : public Napi::AsyncWorker {
public:
    TestAsyncWorker(Napi::Env env) : Napi::AsyncWorker(env), deferred(env) {}
    
    Napi::Promise Promise() { return deferred.Promise(); }
    
protected:
    void Execute() override {
        print_thread_info("N-API AsyncWorker::Execute");
        result = test_function();
    }
    
    void OnOK() override {
        deferred.Resolve(Napi::Number::New(Env(), result));
    }
    
private:
    Napi::Promise::Deferred deferred;
    int result;
};

// N-API methods
Napi::Value TestSync(const Napi::CallbackInfo& info) {
    print_thread_info("N-API Sync Method");
    int result = test_function();
    return Napi::Number::New(info.Env(), result);
}

Napi::Value TestAsync(const Napi::CallbackInfo& info) {
    print_thread_info("N-API Async Method (before queue)");
    auto worker = new TestAsyncWorker(info.Env());
    worker->Queue();
    return worker->Promise();
}

// FFI-compatible version
extern "C" {
    void test_ffi_call() {
        print_thread_info("FFI Direct Call");
        test_function();
    }
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("testSync", Napi::Function::New(env, TestSync));
    exports.Set("testAsync", Napi::Function::New(env, TestAsync));
    return exports;
}

NODE_API_MODULE(thread_comparison, Init)