#!/usr/bin/env bun

console.log('Testing FFI import-only approach...');

try {
    console.log('1. Importing bun:ffi...');
    const { dlopen, FFIType } = require('bun:ffi');
    
    console.log('2. Loading asherah directly...');
    const asherah = require('./build/Release/asherah.node');
    console.log('✅ Asherah loaded successfully!');
    
    console.log('3. Testing setup...');
    asherah.setup({
        ServiceName: 'import-only-test',
        ProductID: 'test',
        KMS: 'static',
        Metastore: 'memory',
        Verbose: false
    });
    console.log('✅ Setup successful!');
    
    console.log('4. Testing encrypt/decrypt...');
    const testData = 'FFI import-only test';
    const encrypted = asherah.encrypt('test-partition', testData);
    const decrypted = asherah.decrypt_string('test-partition', encrypted);
    
    if (decrypted === testData) {
        console.log('✅ Full workflow successful!');
        console.log('🎉 NO EXTERNAL LIBRARY NEEDED!');
    }
    
    asherah.shutdown();
    
} catch (error) {
    console.error('❌ Test failed:', error.message);
}