#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>

int main() {
    printf("Testing dlopen of asherah.node...\n");
    
    const char* path = "./build/Release/asherah.node";
    printf("Opening: %s\n", path);
    
    void* handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        printf("ERROR: %s\n", dlerror());
        return 1;
    }
    
    printf("✅ dlopen succeeded\n");
    
    // Look for napi_module_register
    void* register_fn = dlsym(handle, "napi_module_register");
    if (register_fn) {
        printf("Found napi_module_register at %p\n", register_fn);
    }
    
    dlclose(handle);
    printf("✅ dlclose succeeded\n");
    
    return 0;
}