#include <napi.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

// Forward declare the warmup function from libasherah
extern "C" int32_t WarmupGoRuntime();

// Thread info
void print_thread_info() {
    pid_t tid = syscall(SYS_thread_selfid);
    pthread_t pthread_id = pthread_self();
    fprintf(stderr, "[THREAD] Thread ID: %d, Pthread: %p\n", tid, (void*)pthread_id);
}

// Signal handlers for different signals that might be involved
void signal_handler(int sig, siginfo_t *info, void *context) {
    const char* sig_name = "UNKNOWN";
    switch(sig) {
        case SIGPROF: sig_name = "SIGPROF"; break;
        case SIGALRM: sig_name = "SIGALRM"; break;
        case SIGURG: sig_name = "SIGURG"; break;
        case SIGCHLD: sig_name = "SIGCHLD"; break;
        case SIGPIPE: sig_name = "SIGPIPE"; break;
        case SIGSEGV: sig_name = "SIGSEGV"; break;
        case SIGABRT: sig_name = "SIGABRT"; break;
    }
    fprintf(stderr, "[SIGNAL] Received %s (%d) from PID %d\n", sig_name, sig, info->si_pid);
    
    if (sig == SIGALRM) {
        fprintf(stderr, "[HANG] WarmupGoRuntime appears to be hanging\n");
        exit(1);
    }
}

// Setup signal monitoring
void setup_signal_monitoring() {
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    
    sigaction(SIGPROF, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL); 
    sigaction(SIGURG, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
}

// Library entry point with Go runtime tracing
__attribute__((constructor))
void library_init() {
    fprintf(stderr, "[GO-TRACE] Library constructor called\n");
    print_thread_info();
    
    setup_signal_monitoring();
    
    // Set a timeout in case we hang
    alarm(5);
    
    fprintf(stderr, "[GO-TRACE] About to call WarmupGoRuntime...\n");
    
    // Check if Go runtime might already be initialized
    fprintf(stderr, "[GO-TRACE] Checking pre-call state...\n");
    
    int32_t result = WarmupGoRuntime();
    
    alarm(0); // Cancel timeout
    fprintf(stderr, "[GO-TRACE] WarmupGoRuntime returned: %d\n", result);
    print_thread_info();
}

// Simple test function
Napi::String TestFunction(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::String::New(env, "Go runtime trace successful");
}

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    fprintf(stderr, "[GO-TRACE] Module Init called\n");
    exports.Set(Napi::String::New(env, "test"), Napi::Function::New(env, TestFunction));
    return exports;
}

NODE_API_MODULE(trace_go_runtime, Init)