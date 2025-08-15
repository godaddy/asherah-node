#!/usr/bin/env node

// Test if stack size affects CGO module behavior
console.log('Testing stack size impact on CGO modules...');
console.log('Runtime:', typeof Bun !== 'undefined' ? 'Bun' : 'Node.js');

const asherah = require('./build/Release/asherah.node');
console.log('Module loaded successfully');

const config = {
  ServiceName: 'test-service',
  ProductID: 'test-product', 
  KMS: 'static',
  Metastore: 'memory'
};

console.log('\nTesting async setup...');
const startTime = Date.now();

// This should work in Node.js (8MB stacks) but hang in Bun (smaller stacks)
asherah.setup_async(config).then(() => {
  const elapsed = Date.now() - startTime;
  console.log(`✅ setup_async completed in ${elapsed}ms`);
  console.log('This confirms adequate stack size for CGO operations');
  process.exit(0);
}).catch((error) => {
  console.error('❌ ERROR:', error.message);
  process.exit(1);
});

setTimeout(() => {
  console.error('\n❌ TIMEOUT: setup_async hung after 5 seconds');
  console.error('This suggests insufficient stack size for CGO operations');
  process.exit(1);
}, 5000);