#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <stdint.h>

typedef int32_t (*SetupJson_func)(void* config);

int main() {
    printf("Pure C dlopen test...\n");
    
    // Load the Go module
    void* handle = dlopen("./go_setup_full.node", RTLD_LAZY | RTLD_GLOBAL);
    if (!handle) {
        fprintf(stderr, "Failed to load module: %s\n", dlerror());
        return 1;
    }
    printf("Module loaded\n");
    
    // Find SetupJson
    SetupJson_func SetupJson = (SetupJson_func)dlsym(handle, "SetupJson");
    if (!SetupJson) {
        fprintf(stderr, "Failed to find SetupJson: %s\n", dlerror());
        return 1;
    }
    printf("SetupJson found\n");
    
    // Create Cobhan buffer
    const char* config = "{\"ServiceName\":\"test\",\"ProductID\":\"test\",\"KMS\":\"static\",\"Metastore\":\"memory\"}";
    size_t len = strlen(config);
    size_t alloc_size = len + 8;
    char* buffer = (char*)calloc(1, alloc_size);
    
    int32_t length = (int32_t)len;
    memcpy(buffer, &length, 4);
    memcpy(buffer + 8, config, len);
    
    printf("Calling SetupJson...\n");
    int32_t result = SetupJson(buffer);
    printf("SetupJson returned: %d\n", result);
    
    free(buffer);
    dlclose(handle);
    
    return 0;
}