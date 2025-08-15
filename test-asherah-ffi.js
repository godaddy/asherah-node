#!/usr/bin/env bun

const { dlopen, FFIType, ptr } = require('bun:ffi');

console.log('Testing Asherah via FFI...');

// Try to load SetupJson directly via FFI
try {
    const lib = dlopen('./go_setup_full.node', {
        SetupJson: {
            returns: FFIType.int,
            args: [FFIType.ptr]
        }
    });
    
    console.log('Successfully loaded SetupJson via FFI');
    
    // Create buffer with 8-byte header
    const config = '{"ServiceName":"test","ProductID":"test","KMS":"static","Metastore":"memory"}';
    const configBytes = new TextEncoder().encode(config);
    
    // Allocate buffer with 8-byte header
    const buffer = new ArrayBuffer(configBytes.length + 8);
    const view = new DataView(buffer);
    
    // Write 4-byte length in first 4 bytes (little-endian)
    view.setUint32(0, configBytes.length, true);
    // Bytes 4-7 remain zero (reserved)
    
    // Copy string data starting at offset 8
    const bufferBytes = new Uint8Array(buffer);
    bufferBytes.set(configBytes, 8);
    
    console.log('Calling SetupJson via FFI...');
    console.log('Buffer length:', buffer.byteLength);
    console.log('Config length:', configBytes.length);
    
    // Call SetupJson
    const result = lib.symbols.SetupJson(ptr(bufferBytes));
    console.log('✅ SetupJson via FFI returned:', result);
    
} catch (e) {
    console.error('❌ FFI test failed:', e.message);
}