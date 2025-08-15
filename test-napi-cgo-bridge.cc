#include <napi.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

// Go CGO initialization functions
extern "C" {
    typedef void (*x_cgo_init_func)(void*, void*);
    typedef void (*x_cgo_notify_func)(void);
    
    // These are provided by the Go runtime
    void* _cgo_init = nullptr;
    void* _cgo_notify_runtime_init_done = nullptr;
}

void print_thread_info(const char* context) {
    pthread_t tid = pthread_self();
    fprintf(stderr, "%s - Thread: %p, Main: %s\n", 
            context, (void*)tid, pthread_main_np() ? "YES" : "NO");
}

// Try to initialize CGO manually
void try_cgo_init() {
    fprintf(stderr, "Attempting manual CGO initialization...\n");
    
    // Try to load the Go library and find init functions
    void* handle = dlopen("./lib/libasherah.a", RTLD_LAZY | RTLD_GLOBAL);
    if (handle) {
        fprintf(stderr, "Loaded libasherah.a\n");
        
        x_cgo_init_func cgo_init = (x_cgo_init_func)dlsym(handle, "x_cgo_init");
        if (cgo_init) {
            fprintf(stderr, "Found x_cgo_init, calling it\n");
            cgo_init(nullptr, nullptr);
        } else {
            fprintf(stderr, "x_cgo_init not found\n");
        }
        
        x_cgo_notify_func notify = (x_cgo_notify_func)dlsym(handle, "x_cgo_notify_runtime_init_done");
        if (notify) {
            fprintf(stderr, "Found x_cgo_notify_runtime_init_done, calling it\n");
            notify();
        } else {
            fprintf(stderr, "x_cgo_notify_runtime_init_done not found\n");
        }
    } else {
        fprintf(stderr, "Failed to load libasherah.a: %s\n", dlerror());
    }
}

// External function from Go - we'll load it dynamically
typedef int32_t (*SetupJson_func)(void* config);

// Test calling SetupJson through N-API
Napi::Value TestSetupJsonNAPI(const Napi::CallbackInfo& info) {
    print_thread_info("N-API SetupJson");
    
    // Try manual CGO init first
    try_cgo_init();
    
    const char* config = "{\"ServiceName\":\"test\",\"ProductID\":\"test\",\"KMS\":\"static\",\"Metastore\":\"memory\"}";
    
    // Create Cobhan buffer
    size_t len = strlen(config);
    size_t alloc_size = len + 8;
    char* buffer = (char*)calloc(1, alloc_size);
    
    int32_t length = (int32_t)len;
    memcpy(buffer, &length, 4);
    memcpy(buffer + 8, config, len);
    
    // Load SetupJson dynamically
    void* handle = dlopen("./go_setup_full.node", RTLD_LAZY | RTLD_GLOBAL);
    if (!handle) {
        fprintf(stderr, "Failed to load go_setup_full.node: %s\n", dlerror());
        free(buffer);
        return Napi::Number::New(info.Env(), -1);
    }
    
    SetupJson_func SetupJson = (SetupJson_func)dlsym(handle, "SetupJson");
    if (!SetupJson) {
        fprintf(stderr, "Failed to find SetupJson: %s\n", dlerror());
        free(buffer);
        return Napi::Number::New(info.Env(), -1);
    }
    
    fprintf(stderr, "Calling SetupJson via N-API...\n");
    int32_t result = SetupJson(buffer);
    fprintf(stderr, "SetupJson returned: %d\n", result);
    
    free(buffer);
    return Napi::Number::New(info.Env(), result);
}

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    fprintf(stderr, "N-API module Init called\n");
    print_thread_info("Module Init");
    
    exports.Set("testSetupJson", Napi::Function::New(env, TestSetupJsonNAPI));
    
    return exports;
}

NODE_API_MODULE(test_napi_cgo_bridge, Init)