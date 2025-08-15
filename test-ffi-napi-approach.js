#!/usr/bin/env bun

console.log('=== Testing FFI + N-API Direct Approach ===\n');

const { dlopen, FFIType } = require('bun:ffi');
const path = require('path');

try {
    const asherahPath = path.resolve('./build/Release/asherah.node');
    console.log('1. Loading asherah.node via FFI to call WarmupGoRuntime...');
    
    // Load asherah.node as FFI first
    const asherahFFI = dlopen(asherahPath, {
        WarmupGoRuntime: { returns: FFIType.int32, args: [] }
    });
    
    console.log('2. Calling WarmupGoRuntime via FFI...');
    const warmupResult = asherahFFI.symbols.WarmupGoRuntime();
    console.log(`✅ WarmupGoRuntime returned: ${warmupResult}`);
    
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
    const plaintext = 'FFI+N-API test data';
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
    
    console.log('\n🎉 SUCCESS: FFI + N-API Direct Approach Works!');
    
} catch (error) {
    console.error('❌ FFI + N-API Direct Approach Failed:', error);
    process.exit(1);
}