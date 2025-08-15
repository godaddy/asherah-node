#!/usr/bin/env bun

console.log('Testing minimal C library with bun:ffi...');

const path = require('path');

try {
    const { dlopen, FFIType } = require('bun:ffi');
    
    console.log('1. Loading minimal C library via bun:ffi...');
    const libPath = path.join(__dirname, 'asherah-bun-preload/lib/noop_warmup.dylib');
    const lib = dlopen(libPath, {
        warmup: { returns: FFIType.int, args: [] }
    });
    console.log('✅ C library loaded successfully');
    
    console.log('2. Calling warmup function...');
    const result = lib.symbols.warmup();
    console.log(`✅ Warmup returned: ${result}`);
    
    console.log('3. Loading actual asherah module...');
    const asherah = require('./build/Release/asherah.node');
    console.log('✅ Asherah loaded');
    
    console.log('4. Testing setup...');
    asherah.setup({
        ServiceName: 'c-warmup-test',
        ProductID: 'test', 
        KMS: 'static',
        Metastore: 'memory',
        Verbose: false
    });
    console.log('✅ Setup successful');
    
    const testData = 'C warmup test';
    const encrypted = asherah.encrypt('test-partition', testData);
    const decrypted = asherah.decrypt_string('test-partition', encrypted);
    
    if (decrypted === testData) {
        console.log('✅ Full workflow successful with C warmup!');
        console.log(`📊 Library size: ${require('fs').statSync(libPath).size} bytes`);
    }
    
    asherah.shutdown();
    
} catch (error) {
    console.error('❌ Test failed:', error.message);
}