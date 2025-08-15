#!/usr/bin/env bun

const { dlopen, FFIType, ptr } = require('bun:ffi');

console.log('Testing mock Asherah library...\n');

const lib = dlopen('./test-mock-asherah.dylib', {
    SetEnv: { returns: FFIType.int, args: [FFIType.ptr] },
    SimpleSetupJson: { returns: FFIType.int, args: [FFIType.ptr] },
    SetupJson: { returns: FFIType.int, args: [FFIType.ptr] },
    ComplexSetupJson: { returns: FFIType.int, args: [FFIType.ptr] },
    Shutdown: { returns: FFIType.void, args: [] },
});

function createCobhanBuffer(str) {
    const bytes = new TextEncoder().encode(str);
    const buffer = new ArrayBuffer(bytes.length + 8);
    const view = new DataView(buffer);
    
    // Write length in first 4 bytes (little-endian)
    view.setUint32(0, bytes.length, true);
    // Bytes 4-7 remain zero
    
    // Copy string data
    const bufferBytes = new Uint8Array(buffer);
    bufferBytes.set(bytes, 8);
    
    return bufferBytes;
}

async function testFunction(name, arg) {
    console.log(`\n=== Testing ${name} ===`);
    try {
        const start = Date.now();
        const buffer = arg ? createCobhanBuffer(arg) : null;
        const result = buffer ? lib.symbols[name](ptr(buffer)) : lib.symbols[name]();
        const elapsed = Date.now() - start;
        console.log(`✅ ${name} returned: ${result} (${elapsed}ms)`);
        return true;
    } catch (e) {
        console.error(`❌ ${name} failed: ${e.message}`);
        return false;
    }
}

async function main() {
    // Test SetEnv first
    await testFunction('SetEnv', JSON.stringify({ TEST_VAR: 'test_value' }));
    
    // Test simple setup
    await testFunction('SimpleSetupJson', JSON.stringify({
        ServiceName: 'test',
        ProductID: 'test'
    }));
    
    // Reset
    await testFunction('Shutdown');
    
    // Test normal setup
    await testFunction('SetupJson', JSON.stringify({
        ServiceName: 'test',
        ProductID: 'test',
        KMS: 'static',
        Metastore: 'memory'
    }));
    
    // Reset
    await testFunction('Shutdown');
    
    // Test complex setup
    await testFunction('ComplexSetupJson', JSON.stringify({
        ServiceName: 'test',
        ProductID: 'test'
    }));
    
    console.log('\n=== All tests complete ===');
}

main().catch(console.error);