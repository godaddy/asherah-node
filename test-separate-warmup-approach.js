#!/usr/bin/env bun

console.log('=== Testing Separate Warmup Library Approach ===\n');

const { dlopen, FFIType } = require('bun:ffi');
const path = require('path');

try {
    console.log('1. Loading separate minimal Go warmup library...');
    
    // Load our minimal Go library (separate from asherah.node)
    const warmupPath = path.resolve('./src/bun_warmup_minimal.dylib');
    const warmupLib = dlopen(warmupPath, {
        MinimalWarmup: { returns: FFIType.int, args: [] }
    });
    
    console.log('2. Calling MinimalWarmup...');
    const warmupResult = warmupLib.symbols.MinimalWarmup();
    console.log(`✅ MinimalWarmup returned: ${warmupResult}`);
    
    console.log('3. Now loading asherah.node as N-API module...');
    const asherah = require('./build/Release/asherah.node');
    console.log('✅ N-API module loaded successfully');
    
    console.log('4. Testing full workflow...');
    
    // Setup
    asherah.setup({
        ServiceName: 'test-service',
        ProductID: 'test-product',
        KMS: 'static',
        Metastore: 'memory',
        Verbose: false
    });
    console.log('✅ Setup successful');
    
    // Encrypt/Decrypt
    const plaintext = 'Separate warmup test data';
    const encrypted = asherah.encrypt('test-partition', plaintext);
    const decrypted = asherah.decrypt_string('test-partition', encrypted);
    
    if (decrypted === plaintext) {
        console.log('✅ Encrypt/decrypt successful');
    } else {
        throw new Error('Data mismatch');
    }
    
    // Shutdown
    asherah.shutdown();
    console.log('✅ Shutdown successful');
    
    console.log('\n🎉 SUCCESS: Separate Warmup Library Approach Works!');
    
} catch (error) {
    console.error('❌ Separate Warmup Library Approach Failed:', error);
    process.exit(1);
}