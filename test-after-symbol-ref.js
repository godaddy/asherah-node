#!/usr/bin/env bun

console.log('Testing Bun after symbol reference...');

// First load the symbol reference module
console.log('1. Loading symbol reference module...');
require('./test_symbol_ref.node');
console.log('✅ Symbol reference loaded');

// Now try to load and use the actual asherah module
console.log('2. Loading actual asherah module...');
const asherah = require('./build/Release/asherah.node');
console.log('✅ Asherah module loaded');

console.log('3. Attempting setup...');
try {
    asherah.setup({
        ServiceName: 'after-symbol-ref-test',
        ProductID: 'test',
        KMS: 'static', 
        Metastore: 'memory',
        Verbose: false
    });
    console.log('✅ Setup successful!');
    
    // Try encrypt/decrypt
    const testData = 'Test after symbol reference';
    const encrypted = asherah.encrypt('test-partition', testData);
    const decrypted = asherah.decrypt_string('test-partition', encrypted);
    
    if (decrypted === testData) {
        console.log('✅ Encrypt/decrypt successful!');
    } else {
        console.log('❌ Data mismatch');
    }
    
    asherah.shutdown();
    console.log('✅ Complete workflow successful after symbol reference!');
    
} catch (error) {
    console.error('❌ Failed after symbol reference:', error.message);
}