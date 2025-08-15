#!/usr/bin/env bun

console.log('=== Exploring Minimal FFI Initialization ===\n');

console.log('1. Testing if just importing bun:ffi is enough...');
const importStart = performance.now();
const { dlopen, FFIType } = require('bun:ffi');
const importEnd = performance.now();
console.log(`   Import time: ${(importEnd - importStart).toFixed(2)}ms`);

// Try loading N-API after just import
console.log('2. Testing N-API load after bun:ffi import only...');
try {
    const napiStart = performance.now();
    const asherah = require('./build/Release/asherah.node');
    const napiEnd = performance.now();
    console.log(`   ✅ N-API load successful: ${(napiEnd - napiStart).toFixed(2)}ms`);
    console.log('   🎉 Just importing bun:ffi is sufficient!');
} catch (error) {
    console.log(`   ❌ N-API load failed: ${error.message.split('\n')[0]}`);
    console.log('   ℹ️  Need actual dlopen call...');
}

if (typeof asherah === 'undefined') {
    console.log('\n3. Testing minimal dlopen approaches...');
    
    const path = require('path');
    
    // Test 1: Empty dlopen call
    console.log('   a) Testing empty library dlopen...');
    try {
        const emptyStart = performance.now();
        const emptyLib = dlopen(path.join(__dirname, 'asherah-bun-preload/lib/noop_warmup.dylib'), {});
        const emptyEnd = performance.now();
        console.log(`      Empty dlopen: ${(emptyEnd - emptyStart).toFixed(2)}ms`);
        
        const napiStart = performance.now();
        const asherah = require('./build/Release/asherah.node');
        const napiEnd = performance.now();
        console.log(`      ✅ N-API after empty dlopen: ${(napiEnd - napiStart).toFixed(2)}ms`);
        
    } catch (error) {
        console.log(`      ❌ Empty dlopen failed: ${error.message}`);
    }
}

console.log('\n📊 Analysis: Can we avoid the function definition?');