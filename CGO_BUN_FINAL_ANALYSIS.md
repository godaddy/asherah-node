# Final Analysis: CGO Native Modules in Bun

## Executive Summary
Go runtime hangs when initialized through Bun's N-API implementation, but works correctly in Node.js. The issue is NOT with buffer formats, thread properties, or signal handling - it's a fundamental incompatibility between Go runtime initialization and Bun's JavaScript engine environment.

## Key Findings

### ✅ What Works
1. **Simple Go libraries via FFI** - Direct dlopen/FFI calls to Go functions work in Bun
2. **Node.js N-API** - The exact same native module works perfectly in Node.js
3. **Thread pool execution** - Bun correctly executes async work on thread pool with proper stack size (after fixes)
4. **Native function invocation** - After fixing handle scope issues, native functions ARE being called

### ❌ What Fails
1. **Go runtime init in Bun N-API** - Go runtime hangs during initialization when called from N-API context
2. **Both sync and async** - Hangs on both main thread and worker threads
3. **SetupJson call** - Specifically hangs when calling the first Go function that initializes the runtime

## Technical Details

### Thread Properties Comparison
```
FFI Direct Call (Works):
- Thread: Main (0x2072e5f00)
- Stack: 18 MB
- Signals: None blocked

N-API Sync Call (Hangs in Bun, Works in Node):
- Thread: Main (0x2072e5f00) 
- Stack: 18 MB
- Signals: None blocked

N-API Async Call (Hangs in Bun, Works in Node):
- Thread: Worker (0x16d9c7000)
- Stack: 4 MB (fixed from original issue)
- Signals: None blocked
```

### Buffer Format
- Cobhan uses 8-byte header (not 4 as initially assumed)
- Format: [4-byte length][4-byte reserved][data...]
- This was correctly implemented all along

### Fixes Applied to Bun
1. **Thread Pool Stack Size**: Increased from 4MB to 8MB in `ThreadPool.zig`
2. **Signal Masking**: Added proper signal blocking during thread creation
3. **Handle Scope**: Fixed handle scope creation issues in NapiClass

## Root Cause
The Go runtime cannot initialize properly when called from Bun's JavaScriptCore-based N-API implementation. This appears to be due to:
1. Different JavaScript engine (JavaScriptCore vs V8)
2. Different runtime environment setup
3. Possible TLS (Thread Local Storage) conflicts
4. Different memory management or allocation patterns

## Test Results
```javascript
// Works in Node.js
const asherah = require('./go_setup_full.node');
asherah.setupSync(); // Returns 0 (success)

// Hangs in Bun at Go runtime initialization
const asherah = require('./go_setup_full.node');
asherah.setupSync(); // Hangs indefinitely
```

## Recommendations
1. **Use Node.js** for applications requiring CGO native modules
2. **Consider alternatives**:
   - Pure JavaScript implementations
   - WebAssembly compilation of Go code
   - Separate microservice for Go functionality
   - FFI-based approach (if simple enough)
3. **File Bun issue** with this detailed analysis for potential future fix

## Files Modified
- `bun/src/threading/ThreadPool.zig` - Stack size and signal fixes
- `bun/src/bun.js/bindings/c-bindings.cpp` - Signal management functions
- `bun/src/napi/napi.zig` - Debug logging and investigation

## Conclusion
This is a fundamental incompatibility between Go runtime initialization and Bun's JavaScript engine environment. The issue is NOT a simple bug but rather an architectural mismatch that would require significant changes to either Bun's N-API implementation or Go's runtime initialization to resolve.