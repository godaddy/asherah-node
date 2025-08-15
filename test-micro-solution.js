#!/usr/bin/env bun

console.log('Testing micro C warmup solution...');

try {
    console.log('1. Loading asherah-node with micro C warmup...');
    const asherah = require('./src/index.js');
    console.log('✅ Module loaded');
    
    console.log('2. Testing setup...');
    asherah.setup({
        ServiceName: 'micro-test',
        ProductID: 'test',
        KMS: 'static',
        Metastore: 'memory',
        Verbose: false
    });
    console.log('✅ Setup successful');
    
    const testData = 'Micro warmup test';
    const encrypted = asherah.encrypt('test-partition', testData);
    const decrypted = asherah.decrypt_string('test-partition', encrypted);
    
    if (decrypted === testData) {
        console.log('✅ Full workflow successful!');
        const fs = require('fs');
        const libPath = require('path').join(__dirname, 'asherah-bun-preload/lib/noop_warmup.dylib');
        const libSize = fs.statSync(libPath).size;
        const oldSize = 951426; // Original Go library size
        console.log(`📊 Library size: ${libSize} bytes (${Math.round((oldSize - libSize) / oldSize * 100)}% smaller)`);
    }
    
    asherah.shutdown();
    
} catch (error) {
    console.error('❌ Test failed:', error.message);
    if (error.message.includes('SIGBUS')) {
        console.error('💡 The issue is still with crypto/x509 init in main asherah module');
    }
}