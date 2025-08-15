# BREAKTHROUGH: Go Runtime Works in Bun!

## Critical Discovery
Simple Go shared libraries WORK in Bun when called via FFI/dlopen, but FAIL when called through N-API.

## Test Results

### ✅ What Works
```javascript
// Direct FFI call to Go library - WORKS!
const lib = dlopen('./test-go-init.dylib', {
    TestInit: { returns: FFIType.void, args: [] }
});
lib.symbols.TestInit(); // Successfully initializes Go runtime!
```

### ❌ What Fails
```javascript
// Same Go code called through N-API - HANGS!
asherah.setup_async(config); // Hangs during Go runtime init
```

## The Difference
The issue is NOT a fundamental Go/Bun incompatibility. The problem is specific to:
1. How N-API invokes native functions
2. The environment pointer passed to async execute callbacks
3. Possibly the thread context setup in N-API async work

## Current Investigation
Testing if the napi_env pointer passed to execute callbacks is causing the hang.

## Files Being Modified
- `/bun/src/napi/napi.zig` - Testing different env pointer configurations