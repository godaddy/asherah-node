#!/usr/bin/env bun

const { dlopen, FFIType, ptr } = require('bun:ffi');

console.log('Direct comparison: Working vs Hanging\n');

// Test 1: Simple Go function via FFI (WORKS)
console.log('=== Test 1: Simple Go via FFI ===');
try {
    const simpleLib = dlopen('./test-minimal-hang.dylib', {
        SimpleWork: { returns: FFIType.int, args: [] }
    });
    
    console.log('Calling SimpleWork...');
    const result1 = simpleLib.symbols.SimpleWork();
    console.log('✅ SimpleWork returned:', result1);
} catch (e) {
    console.error('❌ SimpleWork failed:', e.message);
}

// Test 2: Mock Asherah SetupJson via FFI (WORKS)
console.log('\n=== Test 2: Mock Asherah via FFI ===');
try {
    const mockLib = dlopen('./test-mock-asherah.dylib', {
        SetupJson: { returns: FFIType.int, args: [FFIType.ptr] }
    });
    
    const config = JSON.stringify({
        ServiceName: 'test',
        ProductID: 'test'
    });
    
    const bytes = new TextEncoder().encode(config);
    const buffer = new ArrayBuffer(bytes.length + 8);
    const view = new DataView(buffer);
    view.setUint32(0, bytes.length, true);
    const bufferBytes = new Uint8Array(buffer);
    bufferBytes.set(bytes, 8);
    
    console.log('Calling mock SetupJson...');
    const result2 = mockLib.symbols.SetupJson(ptr(bufferBytes));
    console.log('✅ Mock SetupJson returned:', result2);
} catch (e) {
    console.error('❌ Mock SetupJson failed:', e.message);
}

// Test 3: Real Asherah via test module (HANGS)
console.log('\n=== Test 3: Real Asherah via test module ===');
console.log('WARNING: This will hang. Starting in 3 seconds...');
console.log('Press Ctrl+C to stop\n');

setTimeout(() => {
    try {
        const testModule = require('./go_setup_full.node');
        console.log('Test module loaded:', Object.keys(testModule));
        
        console.log('Calling setupSync (THIS WILL HANG)...');
        const result3 = testModule.setupSync();
        console.log('✅ setupSync returned:', result3);
    } catch (e) {
        console.error('❌ setupSync failed:', e.message);
    }
}, 3000);