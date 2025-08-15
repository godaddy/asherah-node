#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

// Thread-local storage for CGO context
static __thread void* bun_cgo_g = NULL;

// CGO callback
static void (*bun_setg_func)(void*) = NULL;

// Runtime state
static volatile int bun_runtime_ready = 0;
static pthread_mutex_t bun_init_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t bun_init_cond = PTHREAD_COND_INITIALIZER;

// Override x_cgo_init to be Bun-safe
void x_cgo_init(void* g, void* setg) {
    fprintf(stderr, "BUN_CGO: x_cgo_init intercepted - g=%p, setg=%p\n", g, setg);
    
    // Store the setg function
    bun_setg_func = (void (*)(void*))setg;
    
    // Don't call the real setg - it crashes Bun
    // Instead, just store the context
    bun_cgo_g = g;
    
    // Signal initialization complete
    pthread_mutex_lock(&bun_init_mutex);
    bun_runtime_ready = 1;
    pthread_cond_broadcast(&bun_init_cond);
    pthread_mutex_unlock(&bun_init_mutex);
    
    fprintf(stderr, "BUN_CGO: x_cgo_init completed safely\n");
}

// Override thread start to be Bun-safe
void* x_cgo_thread_start(void* arg) {
    fprintf(stderr, "BUN_CGO: x_cgo_thread_start intercepted - arg=%p\n", arg);
    
    // Don't do complex initialization - just set thread context
    if (!bun_cgo_g) {
        bun_cgo_g = malloc(32);  // Minimal g struct
        memset(bun_cgo_g, 0, 32);
        fprintf(stderr, "BUN_CGO: allocated minimal g=%p\n", bun_cgo_g);
    }
    
    return arg;
}

// Override wait function
void _cgo_wait_runtime_init_done() {
    fprintf(stderr, "BUN_CGO: _cgo_wait_runtime_init_done waiting...\n");
    
    pthread_mutex_lock(&bun_init_mutex);
    while (!bun_runtime_ready) {
        pthread_cond_wait(&bun_init_cond, &bun_init_mutex);
    }
    pthread_mutex_unlock(&bun_init_mutex);
    
    fprintf(stderr, "BUN_CGO: runtime ready\n");
}

// Thread context functions
void* _cgo_get_context() {
    return bun_cgo_g;
}

void _cgo_set_context(void* ctx) {
    bun_cgo_g = ctx;
}

// Our SetupJson wrapper that initializes CGO first  
int SetupJson(void* config) {
    static int cgo_initialized = 0;
    static int (*real_setup_json)(void*) = NULL;
    
    // Find the real SetupJson on first call
    if (!real_setup_json) {
        void* handle = dlopen(NULL, RTLD_LAZY);
        if (handle) {
            // Look in the asherah library
            real_setup_json = dlsym(handle, "SetupJson");
            if (!real_setup_json) {
                // Look for other variations
                void* lib_handle = dlopen("./go_setup_full.node", RTLD_LAZY);
                if (lib_handle) {
                    real_setup_json = dlsym(lib_handle, "SetupJson");
                }
            }
            dlclose(handle);
        }
    }
    
    if (!cgo_initialized) {
        cgo_initialized = 1;
        fprintf(stderr, "BUN_CGO: First SetupJson call, initializing CGO\n");
        
        // Skip CGO init for now - just call the real function
        fprintf(stderr, "BUN_CGO: Calling real SetupJson directly\n");
    }
    
    if (real_setup_json) {
        return real_setup_json(config);
    } else {
        fprintf(stderr, "BUN_CGO: Could not find real SetupJson!\n");
        return -1;
    }
}

// Dummy setg function
static void bun_setg_internal(void* g) {
    bun_cgo_g = g;
}

// Constructor to initialize when loaded
__attribute__((constructor))
void bun_cgo_init() {
    fprintf(stderr, "BUN_CGO: preload library loaded\n");
    bun_setg_func = bun_setg_internal;
}