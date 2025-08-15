#include <stdio.h>
#include <pthread.h>
#include <sys/resource.h>
#include <unistd.h>

void print_stack_info() {
    struct rlimit rl;
    if (getrlimit(RLIMIT_STACK, &rl) == 0) {
        printf("Process stack limit: ");
        if (rl.rlim_cur == RLIM_INFINITY) {
            printf("unlimited\n");
        } else {
            printf("%zu bytes (%.1f MB)\n", (size_t)rl.rlim_cur, (double)rl.rlim_cur / (1024*1024));
        }
    }
    
    pthread_attr_t attr;
    size_t stack_size;
    
    // Get default thread attributes
    if (pthread_attr_init(&attr) == 0) {
        if (pthread_attr_getstacksize(&attr, &stack_size) == 0) {
            printf("Default pthread stack size: %zu bytes (%.1f MB)\n", 
                   stack_size, (double)stack_size / (1024*1024));
        }
        pthread_attr_destroy(&attr);
    }
}

void* worker_thread(void* arg) {
    char stack_var;
    printf("Worker thread started at stack address: %p\n", &stack_var);
    print_stack_info();
    return NULL;
}

int main() {
    printf("=== Stack Size Analysis ===\n\n");
    
    printf("Main thread:\n");
    print_stack_info();
    
    printf("\nCreating worker thread with default attributes:\n");
    pthread_t thread;
    if (pthread_create(&thread, NULL, worker_thread, NULL) == 0) {
        pthread_join(thread, NULL);
    }
    
    printf("\nCreating worker thread with 8MB stack (libuv style):\n");
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) == 0) {
        size_t stack_size = 8 * 1024 * 1024; // 8MB like libuv
        pthread_attr_setstacksize(&attr, stack_size);
        
        if (pthread_create(&thread, &attr, worker_thread, NULL) == 0) {
            pthread_join(thread, NULL);
        }
        
        pthread_attr_destroy(&attr);
    }
    
    return 0;
}