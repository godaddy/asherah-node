# Go CGO Module Compatibility Fix for Bun

## Summary

This document describes the complete solution for making Go CGO modules work in Bun. The fix requires two separate PRs due to different complexity levels.

## PR 1: N-API Handle Scope Fix (Submitted)

**PR**: https://github.com/oven-sh/bun/pull/21868  
**Status**: Ready for review  
**Diff**: 6 lines changed  

This minimal fix allows Go modules to load by removing the GC sweep phase check that was preventing handle scope creation during module initialization.

### What it fixes:
- Go CGO modules can now be loaded with `require()` without crashing
- Module methods are accessible and enumerable
- No impact on other native modules

### What it doesn't fix:
- Go functions still hang when called (requires PR 2)

## PR 2: CGO WorkPool Thread Registration (Future Work)

**Status**: Not implemented  
**Complexity**: High - requires architecture changes

### Required changes:

1. **Add CGO thread initialization to WorkPool threads**:
   - Modify `src/threading/ThreadPool.zig` to call CGO init
   - Add `bun_init_cgo_thread()` C function to BunProcess.cpp
   - Ensure `x_cgo_thread_start` is called on each WorkPool thread

2. **Handle CGO initialization during dlopen**:
   - Detect and call `x_cgo_init` when loading Go modules
   - Pass proper TLS setup function
   - Call `x_cgo_notify_runtime_init_done` after init

3. **Manage thread-local CGO state**:
   - Track which threads have been initialized
   - Handle thread lifecycle properly
   - Ensure proper cleanup

### Example implementation outline:

```cpp
// In BunProcess.cpp during dlopen:
if (handle) {
    typedef void (*cgo_init_fn)(void*, void (*)(void*));
    cgo_init_fn x_cgo_init = dlsym(handle, "x_cgo_init");
    if (x_cgo_init) {
        x_cgo_init(/*TLS*/, /*setg_gcc function*/);
        // Notify runtime init done...
    }
}

// For WorkPool threads:
extern "C" void bun_init_cgo_thread() {
    typedef void (*cgo_thread_start_fn)(void*);
    cgo_thread_start_fn fn = dlsym(RTLD_DEFAULT, "x_cgo_thread_start");
    if (fn) fn(nullptr);
}
```

## Testing

### Test Go module loading (PR 1):
```javascript
const asherah = require('asherah');
console.log('Module loaded:', Object.getOwnPropertyNames(asherah).length > 0);
// Output: Module loaded: true
```

### Test Go function execution (needs PR 2):
```javascript
const asherah = require('asherah');
asherah.setup(config); // Currently hangs, will work after PR 2
```

## Alternative Approaches Considered

1. **Force synchronous execution**: Doesn't work, creates C++ exception issues
2. **Disable async completely**: Too invasive, breaks other functionality  
3. **Module-specific patches**: Not scalable, needs core fix

## Conclusion

PR 1 is a critical first step that unblocks Go module usage in Bun. While functions don't execute yet, at least modules can load without crashing. PR 2 would complete the solution but requires deeper architectural changes to Bun's threading model.