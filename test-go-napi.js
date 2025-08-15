#!/usr/bin/env bun

const { dlopen, FFIType } = require('bun:ffi');

console.log('=== Testing Go Function: FFI vs N-API ===\n');

// Test 1: FFI direct call
console.log('1. Testing FFI direct call to Go...');
try {
    const lib = dlopen('./go_napi.node', {
        test_go_ffi: {
            returns: FFIType.void,
            args: []
        }
    });
    
    console.log('Calling test_go_ffi...');
    lib.symbols.test_go_ffi();
    console.log('✅ FFI Go call succeeded\n');
} catch (e) {
    console.error('❌ FFI Go call failed:', e.message, '\n');
}

// Test 2: N-API module
console.log('2. Testing N-API calls to Go...');
try {
    const addon = require('./go_napi.node');
    
    console.log('2a. Sync call to Go:');
    const syncResult = addon.testGoSync();
    console.log(`✅ Sync Go result: ${syncResult}\n`);
    
    console.log('2b. Async call to Go:');
    addon.testGoAsync().then(result => {
        console.log(`✅ Async Go result: ${result}\n`);
        console.log('All tests completed successfully!');
    }).catch(e => {
        console.error('❌ Async Go failed:', e.message, '\n');
    });
    
    // Give async time to complete
    setTimeout(() => {
        console.log('Timeout reached - async may have hung');
    }, 5000);
} catch (e) {
    console.error('❌ N-API module failed:', e.message);
}