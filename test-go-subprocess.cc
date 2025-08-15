#include <napi.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

// External Go function that hangs in Bun
extern "C" int SetupJson(void* config);

// Create a subprocess that can run Go functions safely
Napi::Value TestGoSubprocess(const Napi::CallbackInfo& info) {
    fprintf(stderr, "Creating subprocess to run Go function...\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process - run the Go function
        fprintf(stderr, "Child: Attempting Go function call...\n");
        
        char buffer[16] = {0};
        *(int32_t*)buffer = 2;
        strcpy(buffer + 8, "{}");
        
        int result = SetupJson(buffer);
        fprintf(stderr, "Child: Go function returned %d\n", result);
        exit(result == 0 ? 0 : 1);
    } else if (pid > 0) {
        // Parent process - wait for result
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            fprintf(stderr, "Parent: Child exited with code %d\n", exit_code);
            return Napi::Number::New(info.Env(), exit_code);
        } else {
            fprintf(stderr, "Parent: Child did not exit normally\n");
            return Napi::Number::New(info.Env(), -1);
        }
    } else {
        // Fork failed
        fprintf(stderr, "Fork failed\n");
        return Napi::Number::New(info.Env(), -1);
    }
}

// Test with pre-initialization
Napi::Value TestGoPreInit(const Napi::CallbackInfo& info) {
    fprintf(stderr, "Testing Go with pre-initialization...\n");
    
    // Try to initialize Go runtime step by step
    fprintf(stderr, "Step 1: Basic setup...\n");
    
    char buffer[16] = {0};
    *(int32_t*)buffer = 2;
    strcpy(buffer + 8, "{}");
    
    fprintf(stderr, "Step 2: About to call SetupJson...\n");
    
    // This will hang in Bun but work in subprocess
    int result = SetupJson(buffer);
    
    fprintf(stderr, "Step 3: SetupJson returned %d\n", result);
    return Napi::Number::New(info.Env(), result);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("subprocess", Napi::Function::New(env, TestGoSubprocess));
    exports.Set("preinit", Napi::Function::New(env, TestGoPreInit));
    return exports;
}

NODE_API_MODULE(go_subprocess, Init)