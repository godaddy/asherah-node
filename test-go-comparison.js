#!/usr/bin/env bun

const { dlopen, FFIType, ptr } = require('bun:ffi');

console.log('=== Comparing Go Libraries ===\n');

// Test 1: Simple Go library (known to work)
console.log('1. Testing simple Go library (test-go-init.dylib)...');
try {
    const simpleLib = dlopen('./test-go-init.dylib', {
        TestInit: {
            returns: FFIType.void,
            args: []
        }
    });
    
    console.log('Calling TestInit...');
    simpleLib.symbols.TestInit();
    console.log('✅ Simple Go library succeeded\n');
} catch (e) {
    console.error('❌ Simple Go library failed:', e.message, '\n');
}

// Test 2: Complex Go library (libasherah.a via FFI)
console.log('2. Testing complex Go library (libasherah via FFI)...');
try {
    // Try to load SetupJson directly from go_napi.node
    const complexLib = dlopen('./go_napi.node', {
        test_go_ffi: {
            returns: FFIType.void,
            args: []
        }
    });
    
    console.log('Calling test_go_ffi (which calls SetupJson)...');
    const timeout = setTimeout(() => {
        console.log('⏰ Call appears to be hanging (3 seconds)...');
        process.exit(1);
    }, 3000);
    
    complexLib.symbols.test_go_ffi();
    clearTimeout(timeout);
    console.log('✅ Complex Go library succeeded\n');
} catch (e) {
    console.error('❌ Complex Go library failed:', e.message, '\n');
}