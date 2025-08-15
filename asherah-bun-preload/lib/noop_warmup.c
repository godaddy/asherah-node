// Minimal C library for Bun runtime warmup
// Just needs to be loadable to trigger FFI subsystem initialization

__attribute__((visibility("default")))
int warmup(void) {
    return 1;
}