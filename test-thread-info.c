#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void print_thread_info(const char* context) {
    pthread_t tid = pthread_self();
    
    // Get stack size
    pthread_attr_t attr;
    size_t stack_size = 0;
    void* stack_addr = NULL;
    
    // macOS-specific way to get stack info
    stack_size = pthread_get_stacksize_np(tid);
    stack_addr = pthread_get_stackaddr_np(tid);
    
    // Get signal mask
    sigset_t mask;
    pthread_sigmask(SIG_BLOCK, NULL, &mask);
    
    printf("\n=== Thread Info: %s ===\n", context);
    printf("Thread ID: %p\n", (void*)tid);
    printf("Stack size: %zu bytes (%.2f MB)\n", stack_size, stack_size / (1024.0 * 1024.0));
    printf("Stack address: %p\n", stack_addr);
    printf("Process ID: %d\n", getpid());
    
    // Check important signals
    printf("Signal mask:\n");
    printf("  SIGPROF: %s\n", sigismember(&mask, SIGPROF) ? "blocked" : "unblocked");
    printf("  SIGPIPE: %s\n", sigismember(&mask, SIGPIPE) ? "blocked" : "unblocked");
    printf("  SIGALRM: %s\n", sigismember(&mask, SIGALRM) ? "blocked" : "unblocked");
    printf("  SIGUSR1: %s\n", sigismember(&mask, SIGUSR1) ? "blocked" : "unblocked");
    printf("  SIGUSR2: %s\n", sigismember(&mask, SIGUSR2) ? "blocked" : "unblocked");
    
    fflush(stdout);
}

// Export for N-API
__attribute__((visibility("default")))
void check_thread_info(const char* context) {
    print_thread_info(context);
}