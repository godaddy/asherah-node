#include <napi.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Go types
typedef unsigned char GoUint8;
typedef int GoInt32;

// SetupJson with output buffer
extern "C" GoInt32 SetupJson(void* config);

// Create Cobhan buffer with 8-byte header
void* create_cobhan_buffer(const char* str, size_t* total_size) {
    size_t len = strlen(str);
    size_t alloc_size = len + 8;  // 8-byte header for Cobhan-go
    char* buffer = (char*)calloc(1, alloc_size);  // Zero-initialize
    
    // Write 4-byte length in first 4 bytes (little-endian)
    int32_t length = (int32_t)len;
    memcpy(buffer, &length, 4);
    // Bytes 4-7 remain zero (reserved/padding)
    
    // Copy string data starting at offset 8
    memcpy(buffer + 8, str, len);
    
    if (total_size) *total_size = alloc_size;
    return buffer;
}

void print_thread_info(const char* context) {
    pthread_t tid = pthread_self();
    fprintf(stderr, "%s - Thread: %p, Main: %s\n", 
            context, (void*)tid, pthread_main_np() ? "YES" : "NO");
}

// Test sync call
Napi::Value TestSetupSync(const Napi::CallbackInfo& info) {
    print_thread_info("Sync SetupJson");
    
    const char* config = "{\"ServiceName\":\"test\",\"ProductID\":\"test\",\"KMS\":\"static\",\"Metastore\":\"memory\"}";
    
    size_t size;
    void* buffer = create_cobhan_buffer(config, &size);
    
    // Debug: print first 16 bytes
    unsigned char* bytes = (unsigned char*)buffer;
    fprintf(stderr, "Buffer contents (first 16 bytes): ");
    for (int i = 0; i < 16 && i < size; i++) {
        fprintf(stderr, "%02x ", bytes[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "Buffer as string from offset 8: %.20s...\n", (char*)buffer + 8);
    
    fprintf(stderr, "Calling SetupJson with %zu byte buffer...\n", size);
    GoInt32 result = SetupJson(buffer);
    fprintf(stderr, "SetupJson returned: %d\n", result);
    
    free(buffer);
    return Napi::Number::New(info.Env(), result);
}

// Async worker
class SetupAsyncWorker : public Napi::AsyncWorker {
public:
    SetupAsyncWorker(Napi::Env env) : Napi::AsyncWorker(env), deferred(env) {}
    
    Napi::Promise Promise() { return deferred.Promise(); }
    
protected:
    void Execute() override {
        print_thread_info("Async SetupJson");
        
        const char* config = "{\"ServiceName\":\"test\",\"ProductID\":\"test\",\"KMS\":\"static\",\"Metastore\":\"memory\"}";
        
        size_t size;
        void* buffer = create_cobhan_buffer(config, &size);
        
        fprintf(stderr, "Async: Calling SetupJson...\n");
        result = SetupJson(buffer);
        fprintf(stderr, "Async: SetupJson returned: %d\n", result);
        
        free(buffer);
    }
    
    void OnOK() override {
        deferred.Resolve(Napi::Number::New(Env(), result));
    }
    
private:
    Napi::Promise::Deferred deferred;
    GoInt32 result;
};

Napi::Value TestSetupAsync(const Napi::CallbackInfo& info) {
    auto worker = new SetupAsyncWorker(info.Env());
    worker->Queue();
    return worker->Promise();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("setupSync", Napi::Function::New(env, TestSetupSync));
    exports.Set("setupAsync", Napi::Function::New(env, TestSetupAsync));
    return exports;
}

NODE_API_MODULE(go_setup_full, Init)