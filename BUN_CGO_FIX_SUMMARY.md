# Bun CGO Native Module Fix Summary

## Problem
Native modules using CGO (Go runtime) were completely freezing in Bun but working correctly in Node.js. The freeze occurred during both synchronous and asynchronous native function calls.

## Root Causes Identified

### 1. Thread Pool Signal Masking
Bun's ThreadPool was not properly masking signals during thread creation, causing Go runtime initialization to fail on worker threads.

### 2. Insufficient Stack Size
Bun's ThreadPool allocated only 4MB stack size compared to Node.js's 8MB, insufficient for Go runtime requirements.

### 3. Handle Scope Creation Issues
JavaScriptCore's handle scope creation was causing hangs when called from native module context on the main thread.

## Fixes Applied

### 1. Signal Masking in ThreadPool (FIXED)
**File:** `bun/src/threading/ThreadPool.zig`
- Added signal masking during thread creation
- Blocks all signals before pthread_create, unblocks after
- Ensures clean signal state for Go runtime initialization

**File:** `bun/src/bun.js/bindings/c-bindings.cpp`
- Added `bun_block_signals_for_thread_creation()` and `bun_unblock_signals_after_thread_creation()`
- Uses pthread_sigmask to manage signal sets

### 2. Stack Size Increase (FIXED)
**File:** `bun/src/threading/ThreadPool.zig`
- Changed stack size from 4MB to 8MB to match Node.js
- Ensures sufficient stack space for Go runtime

### 3. Main Thread Signal Handling (FIXED)
**File:** `bun/src/main.zig`
- Added signal unblocking in main() to ensure all signals are unblocked
- Matches Node.js behavior for main thread signal handling

### 4. Handle Scope Bypass (PARTIAL)
**File:** `bun/src/bun.js/bindings/NapiClass.cpp`
- Disabled NapiHandleScope creation before native function calls
- Allows native functions to be called without JavaScriptCore hang

**File:** `bun/src/bun.js/bindings/napi_handle_scope.cpp`
- Modified to skip first few handle scope creations for runtime initialization

## Current Status

### Working ✅
- Async native module functions work correctly
- Module loading succeeds
- Native functions are now called (previously hung before invocation)
- Thread pool properly initialized for Go runtime

### Still Broken ❌
- Synchronous native module calls on main thread hang when Go runtime initializes
- The hang occurs specifically when the Go function is first called from the main thread
- Go's runtime initialization appears incompatible with Bun's main thread environment

## Technical Details

The investigation revealed that:
1. Go runtime requires specific signal handling and stack configuration
2. JavaScriptCore's garbage collection mechanisms conflict with Go's runtime initialization
3. The issue is NOT in the N-API layer itself, but in the interaction between JavaScriptCore and Go runtime

## Recommendations

1. **For Module Authors:** Use async functions exclusively when using CGO modules with Bun
2. **For Bun Team:** Consider implementing a dedicated native module thread pool separate from the main thread
3. **Alternative Approach:** Investigate running all native module synchronous calls on a separate thread with proper callback mechanisms

## Files Modified

1. `/bun/src/threading/ThreadPool.zig` - Signal masking and stack size
2. `/bun/src/bun.js/bindings/c-bindings.cpp` - Signal management functions
3. `/bun/src/main.zig` - Main thread signal initialization
4. `/bun/src/bun.js/bindings/NapiClass.cpp` - Handle scope bypass
5. `/bun/src/bun.js/bindings/napi_handle_scope.cpp` - Runtime initialization workaround

## Test Results

- ✅ Async CGO functions work after fixes
- ❌ Sync CGO functions still hang on main thread
- ✅ Native functions are now invoked (progress from complete freeze)
- ✅ Module loading and initialization successful

## Next Steps

Further investigation needed:
1. Why Go runtime initialization fails on Bun's main thread specifically
2. Potential JavaScriptCore vs V8 differences in thread local storage
3. Possibility of running sync calls through async mechanism transparently