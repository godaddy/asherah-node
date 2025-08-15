#include <stdio.h>
#include <dlfcn.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

// Function pointer types for CGO functions
typedef int (*SetupJsonFunc)(void* configJson);

// Signal handler to catch any signals during Go runtime init
volatile sig_atomic_t signal_received = 0;
int last_signal = 0;

void signal_handler(int sig) {
    signal_received = 1;
    last_signal = sig;
    printf("📡 Signal %d received during Go runtime init\n", sig);
}

// Install signal handlers for common signals that might cause hanging
void install_signal_handlers() {
    signal(SIGPROF, signal_handler);
    signal(SIGALRM, signal_handler);
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
}

// Timeout handler
volatile sig_atomic_t timeout_occurred = 0;
void timeout_handler(int sig) {
    timeout_occurred = 1;
    printf("⏰ TIMEOUT: Go runtime init took too long\n");
}

int main() {
    printf("=== Go Runtime Initialization Trace ===\n");
    printf("PID: %d\n", getpid());
    printf("Main thread: %p\n", (void*)pthread_self());
    
    // Install signal handlers to monitor what's happening
    install_signal_handlers();
    
    // Set up timeout
    signal(SIGALRM, timeout_handler);
    alarm(10); // 10 second timeout
    
    // Load the native library
    printf("\n🔧 Loading native library...\n");
    void* handle = dlopen("./build/Release/asherah.node", RTLD_LAZY);
    if (!handle) {
        printf("❌ Failed to load library: %s\n", dlerror());
        return 1;
    }
    printf("✅ Library loaded successfully\n");
    
    // Get the SetupJson function
    printf("\n🔍 Looking up SetupJson function...\n");
    SetupJsonFunc setup_func = (SetupJsonFunc)dlsym(handle, "SetupJson");
    if (!setup_func) {
        printf("❌ Failed to find SetupJson: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }
    printf("✅ SetupJson function found at %p\n", (void*)setup_func);
    
    // Prepare minimal config
    const char* config = "{\"ServiceName\":\"test\",\"ProductID\":\"test\",\"KMS\":\"static\",\"Metastore\":\"memory\"}";
    printf("\n📋 Config: %s\n", config);
    
    printf("\n🚀 Calling SetupJson - this is where it might hang...\n");
    printf("Signal status before call: received=%d, last=%d\n", signal_received, last_signal);
    
    // Make the call that causes the hang
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    int result = setup_func((void*)config);
    
    gettimeofday(&end, NULL);
    long elapsed_ms = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
    
    // If we get here, it didn't hang
    alarm(0); // Cancel timeout
    printf("✅ SetupJson completed!\n");
    printf("Result: %d\n", result);
    printf("Elapsed time: %ld ms\n", elapsed_ms);
    printf("Signal status after call: received=%d, last=%d\n", signal_received, last_signal);
    
    if (timeout_occurred) {
        printf("⚠️  Timeout occurred during call\n");
    }
    
    // Clean up
    dlclose(handle);
    printf("\n🧹 Cleanup complete\n");
    
    return 0;
}