#include <stdio.h>
#include <dlfcn.h>

// Declare the Go function
extern "C" {
    typedef long long GoInt64;
    typedef GoInt64 GoInt;
    typedef unsigned char GoUint8;
    
    struct CBuffer {
        GoInt64 length;
        GoUint8* data;
    };
    
    // This is the actual Go function from libasherah.a
    extern GoInt SetupJson(CBuffer config);
}

int main() {
    printf("Testing direct Go function call...\n");
    
    // Create a minimal config
    const char* config = "{}";
    CBuffer buffer;
    buffer.length = 2;
    buffer.data = (GoUint8*)config;
    
    printf("Calling SetupJson...\n");
    GoInt result = SetupJson(buffer);
    printf("SetupJson returned: %lld\n", result);
    
    return 0;
}