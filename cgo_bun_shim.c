#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Thread-local storage for CGO context
static __thread void* cgo_g_context = NULL;

// CGO callback function pointer
static void (*cgo_setg_func)(void*) = NULL;

// Runtime initialization state
static volatile int runtime_initialized = 0;
static pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t init_cond = PTHREAD_COND_INITIALIZER;

// CGO initialization function
void x_cgo_init(void* g, void* setg) {
    fprintf(stderr, "x_cgo_init: g=%p, setg=%p\n", g, setg);
    
    cgo_setg_func = (void (*)(void*))setg;
    
    // Set the initial context
    if (cgo_setg_func && g) {
        cgo_g_context = g;
        cgo_setg_func(g);
    }
    
    // Signal that CGO is initialized
    pthread_mutex_lock(&init_mutex);
    runtime_initialized = 1;
    pthread_cond_broadcast(&init_cond);
    pthread_mutex_unlock(&init_mutex);
    
    fprintf(stderr, "x_cgo_init: completed\n");
}

// Wait for runtime initialization
void _cgo_wait_runtime_init_done() {
    fprintf(stderr, "_cgo_wait_runtime_init_done: waiting...\n");
    
    pthread_mutex_lock(&init_mutex);
    while (!runtime_initialized) {
        pthread_cond_wait(&init_cond, &init_mutex);
    }
    pthread_mutex_unlock(&init_mutex);
    
    fprintf(stderr, "_cgo_wait_runtime_init_done: done\n");
}

// Thread start function for CGO
void* x_cgo_thread_start(void* arg) {
    fprintf(stderr, "x_cgo_thread_start: arg=%p\n", arg);
    
    // Initialize thread-local context
    if (cgo_setg_func) {
        void* new_g = malloc(1024);  // Allocate space for Go's g struct
        memset(new_g, 0, 1024);
        cgo_g_context = new_g;
        cgo_setg_func(new_g);
        fprintf(stderr, "x_cgo_thread_start: set g=%p\n", new_g);
    }
    
    return arg;
}

// Get current CGO context
void* _cgo_get_context() {
    return cgo_g_context;
}

// Set CGO context
void _cgo_set_context(void* ctx) {
    cgo_g_context = ctx;
}