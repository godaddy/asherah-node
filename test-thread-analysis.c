#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/resource.h>
#include <errno.h>
#include <string.h>
#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/thread_info.h>
#include <mach/mach_types.h>
#endif

void print_signal_mask(const char* context) {
    sigset_t current_mask;
    int result = pthread_sigmask(SIG_BLOCK, NULL, &current_mask);
    if (result != 0) {
        printf("%s signal mask: Error %d (%s)\n", context, result, strerror(result));
        return;
    }
    
    printf("%s signal mask:\n", context);
    printf("  SIGTERM: %s\n", sigismember(&current_mask, SIGTERM) ? "BLOCKED" : "unblocked");
    printf("  SIGINT:  %s\n", sigismember(&current_mask, SIGINT) ? "BLOCKED" : "unblocked");
    printf("  SIGQUIT: %s\n", sigismember(&current_mask, SIGQUIT) ? "BLOCKED" : "unblocked");
    printf("  SIGPROF: %s\n", sigismember(&current_mask, SIGPROF) ? "BLOCKED" : "unblocked");
    printf("  SIGALRM: %s\n", sigismember(&current_mask, SIGALRM) ? "BLOCKED" : "unblocked");
    printf("  SIGUSR1: %s\n", sigismember(&current_mask, SIGUSR1) ? "BLOCKED" : "unblocked");
    printf("  SIGUSR2: %s\n", sigismember(&current_mask, SIGUSR2) ? "BLOCKED" : "unblocked");
}

void print_thread_attributes() {
    pthread_t self = pthread_self();
    
#ifdef __APPLE__
    // Try to get stack bounds on macOS
    void* stack_addr = pthread_get_stackaddr_np(self);
    size_t stack_size = pthread_get_stacksize_np(self);
    printf("Thread stack: addr=%p, size=%zu bytes (%.1f MB)\n", 
           stack_addr, stack_size, stack_size / (1024.0 * 1024.0));
#else
    // Linux version
    pthread_attr_t attr;
    int result = pthread_getattr_np(self, &attr);
    if (result != 0) {
        printf("Thread attributes: Error %d (%s)\n", result, strerror(result));
        return;
    }
    
    void* stack_addr;
    size_t stack_size;
    result = pthread_attr_getstack(&attr, &stack_addr, &stack_size);
    if (result == 0) {
        printf("Thread stack: addr=%p, size=%zu bytes (%.1f MB)\n", 
               stack_addr, stack_size, stack_size / (1024.0 * 1024.0));
    } else {
        printf("Thread stack: Error %d (%s)\n", result, strerror(result));
    }
    
    pthread_attr_destroy(&attr);
#endif
}

void print_resource_limits() {
    struct rlimit rlim;
    
    if (getrlimit(RLIMIT_STACK, &rlim) == 0) {
        printf("Stack limit: ");
        if (rlim.rlim_cur == RLIM_INFINITY) {
            printf("unlimited");
        } else {
            printf("%llu bytes (%.1f MB)", (unsigned long long)rlim.rlim_cur, 
                   rlim.rlim_cur / (1024.0 * 1024.0));
        }
        printf(" / ");
        if (rlim.rlim_max == RLIM_INFINITY) {
            printf("unlimited\n");
        } else {
            printf("%llu bytes (%.1f MB)\n", (unsigned long long)rlim.rlim_max,
                   rlim.rlim_max / (1024.0 * 1024.0));
        }
    }
    
    if (getrlimit(RLIMIT_AS, &rlim) == 0) {
        printf("Address space limit: ");
        if (rlim.rlim_cur == RLIM_INFINITY) {
            printf("unlimited");
        } else {
            printf("%llu bytes", (unsigned long long)rlim.rlim_cur);
        }
        printf(" / ");
        if (rlim.rlim_max == RLIM_INFINITY) {
            printf("unlimited\n");
        } else {
            printf("%llu bytes\n", (unsigned long long)rlim.rlim_max);
        }
    }
}

int main() {
    printf("=== Thread Analysis Tool ===\n");
    printf("PID: %d\n", getpid());
    printf("Main thread ID: %p\n", (void*)pthread_self());
    
    printf("\n");
    print_signal_mask("Main thread");
    
    printf("\n");
    print_thread_attributes();
    
    printf("\n");
    print_resource_limits();
    
    return 0;
}