#!/usr/bin/env bun

console.log('Testing current asherah-node module in Bun (should hang)...\n');

// First warm up Go runtime
console.log('Warming up Go runtime...');
require('./src/bun_warmup.js');
console.log('Warmup complete, loading asherah...\n');

const asherah = require('./build/Release/asherah.node');

console.log('Module loaded, setting up...');

// This should hang in Bun if not fixed
asherah.setup({
    ServiceName: 'test',
    ProductID: 'test', 
    KMS: 'static',
    Metastore: 'memory',
    Verbose: true
});

console.log('✅ Setup completed - module is fixed!');

// Test encrypt/decrypt
const encrypted = asherah.encrypt('partition1', 'test data');
console.log('✅ Encrypted data');

const decrypted = asherah.decrypt_string('partition1', encrypted);
console.log('✅ Decrypted:', decrypted);

asherah.shutdown();
console.log('✅ Shutdown complete');