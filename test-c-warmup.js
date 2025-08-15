#!/usr/bin/env bun

console.log('Testing minimal C library warmup...');

const path = require('path');
const libPath = path.join(__dirname, 'asherah-bun-preload/lib/noop_warmup.dylib');

try {
    console.log('1. Loading minimal C library...');
    const lib = Bun.dlopen(libPath, {
        warmup: { args: [], returns: "int" }
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
    }
    
    asherah.shutdown();
    
} catch (error) {
    console.error('❌ Test failed:', error.message);
}