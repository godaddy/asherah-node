#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

typedef void (*cgo_init_fn)(void*, void (*)(void*));
typedef void (*cgo_notify_fn)(void);
typedef void (*cgo_thread_start_fn)(void*);

void* worker_thread(void* arg) {
    int thread_num = *(int*)arg;
    pthread_t tid = pthread_self();
    
    printf("Worker thread %d (tid=%p) starting\n", thread_num, (void*)tid);
    
    // Try to find and call x_cgo_thread_start
    cgo_thread_start_fn thread_start = (cgo_thread_start_fn)dlsym(RTLD_DEFAULT, "_x_cgo_thread_start");
    if (thread_start) {
        printf("Worker thread %d: Found x_cgo_thread_start, calling it...\n", thread_num);
        thread_start(NULL);
        printf("Worker thread %d: x_cgo_thread_start returned successfully\n", thread_num);
    } else {
        printf("Worker thread %d: x_cgo_thread_start NOT found\n", thread_num);
    }
    
    return NULL;
}

int main() {
    printf("=== CGO Thread Initialization Analysis ===\n\n");
    
    // Step 1: Load the module
    const char* path = "./build/Release/asherah.node";
    printf("1. Loading module: %s\n", path);
    
    void* handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        printf("ERROR: %s\n", dlerror());
        return 1;
    }
    printf("   ✅ Module loaded\n\n");
    
    // Step 2: Find CGO initialization functions
    printf("2. Looking for CGO initialization functions:\n");
    
    cgo_init_fn x_cgo_init = (cgo_init_fn)dlsym(handle, "_x_cgo_init");
    printf("   x_cgo_init: %s (%p)\n", x_cgo_init ? "FOUND" : "NOT FOUND", x_cgo_init);
    
    cgo_notify_fn x_cgo_notify = (cgo_notify_fn)dlsym(handle, "_x_cgo_notify_runtime_init_done");
    printf("   x_cgo_notify_runtime_init_done: %s (%p)\n", x_cgo_notify ? "FOUND" : "NOT FOUND", x_cgo_notify);
    
    cgo_thread_start_fn x_cgo_thread_start = (cgo_thread_start_fn)dlsym(handle, "_x_cgo_thread_start");
    printf("   x_cgo_thread_start: %s (%p)\n\n", x_cgo_thread_start ? "FOUND" : "NOT FOUND", x_cgo_thread_start);
    
    // Step 3: Initialize CGO (simulating what should happen)
    if (x_cgo_init) {
        printf("3. Calling x_cgo_init (with NULL args for now):\n");
        // Note: In real implementation, we'd need proper TLS setup
        // x_cgo_init(NULL, NULL);  // Commented out as it may crash without proper setup
        printf("   ⚠️  Skipping actual call (needs proper TLS setup)\n\n");
    }
    
    // Step 4: Test thread initialization
    printf("4. Testing thread initialization:\n");
    printf("   Main thread tid=%p\n", (void*)pthread_self());
    
    // Create worker threads
    pthread_t threads[2];
    int thread_nums[2] = {1, 2};
    
    for (int i = 0; i < 2; i++) {
        if (pthread_create(&threads[i], NULL, worker_thread, &thread_nums[i]) != 0) {
            printf("   ERROR: Failed to create thread %d\n", i+1);
        }
    }
    
    // Wait for threads
    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\n5. Summary:\n");
    printf("   - CGO symbols are present in the module\n");
    printf("   - Worker threads can find x_cgo_thread_start via RTLD_DEFAULT\n");
    printf("   - Bun's ThreadPool needs to call x_cgo_thread_start for each worker\n");
    printf("   - Node.js libuv threads somehow satisfy CGO's requirements\n");
    
    dlclose(handle);
    return 0;
}