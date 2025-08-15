#!/usr/bin/env bun

console.log('Investigating what makes Asherah different...\n');

// Test 1: Simple FFI call to Asherah SetupJson  
console.log('=== Test 1: FFI Direct Call to SetupJson ===');
try {
    const { dlopen, FFIType, ptr } = require('bun:ffi');
    
    // Load Asherah library directly via FFI
    const asherahLib = dlopen('./go_setup_full.node', {
        SetupJson: {
            returns: FFIType.int,
            args: [FFIType.ptr]
        }
    });
    
    console.log('Asherah library loaded via FFI');
    
    // Create test config
    const config = JSON.stringify({
        ServiceName: "test",
        ProductID: "test", 
        KMS: "static",
        Metastore: "memory"
    });
    
    console.log('Config created, calling SetupJson via FFI...');
    console.log('This should hang...');
    
    // This call should hang
    const result = asherahLib.symbols.SetupJson(ptr(new TextEncoder().encode(config)));
    console.log('SetupJson returned:', result);
    
} catch (e) {
    console.error('FFI test failed:', e.message);
}

console.log('\n=== Test 2: Process Information ===');
console.log('PID:', process.pid);
console.log('Platform:', process.platform);
console.log('Architecture:', process.arch);
console.log('Node version:', process.version);

console.log('\nIf you see this message, Asherah did NOT hang');
process.exit(0);