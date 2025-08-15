#!/usr/bin/env bun

console.log('Testing embedded warmup approach...\n');

// Load module that warms up Go runtime during its Init
console.log('Loading module with embedded warmup...');
const warmupModule = require('./test-embedded-warmup.node');
console.log('Warmup module loaded:', warmupModule.test());

// Now try loading Asherah
console.log('\nLoading Asherah after embedded warmup...');
const asherah = require('./dist/asherah');
console.log('✅ Asherah loaded!');

const config = {
    ServiceName: "test",
    ProductID: "test",
    KMS: "static",
    Metastore: "memory"
};

console.log('\nCalling setup...');
asherah.setup(config);
console.log('✅ Setup worked!');

asherah.shutdown();
console.log('✅ Test complete');