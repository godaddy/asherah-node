#!/usr/bin/env bun

const { dlopen, FFIType } = require('bun:ffi');

console.log('=== Testing Thread Properties: FFI vs N-API ===\n');

// Test 1: FFI direct call
console.log('1. Testing FFI direct call...');
try {
    const lib = dlopen('./build/Release/thread_comparison.node', {
        test_ffi_call: {
            returns: FFIType.void,
            args: []
        }
    });
    lib.symbols.test_ffi_call();
    console.log('✅ FFI call succeeded\n');
} catch (e) {
    console.error('❌ FFI call failed:', e.message, '\n');
}

// Test 2: N-API module
console.log('2. Testing N-API module...');
try {
    const addon = require('./build/Release/thread_comparison.node');
    
    console.log('2a. Sync call:');
    const syncResult = addon.testSync();
    console.log(`✅ Sync result: ${syncResult}\n`);
    
    console.log('2b. Async call:');
    addon.testAsync().then(result => {
        console.log(`✅ Async result: ${result}\n`);
    }).catch(e => {
        console.error('❌ Async failed:', e.message, '\n');
    });
    
    // Give async time to complete
    setTimeout(() => {
        console.log('Test complete.');
    }, 1000);
} catch (e) {
    console.error('❌ N-API module failed:', e.message);
}