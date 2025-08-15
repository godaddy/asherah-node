#!/usr/bin/env bun

console.log('Testing WITHOUT any preload or warmup...\n');

// Directly load Asherah - this should hang
console.log('Loading Asherah directly (no warmup)...');
const asherah = require('./dist/asherah');

console.log('Asherah loaded!');

const config = {
    ServiceName: "test",
    ProductID: "test",
    KMS: "static",
    Metastore: "memory"
};

console.log('Calling setup...');
asherah.setup(config);
console.log('✅ Setup worked!');

asherah.shutdown();
console.log('✅ Test complete');