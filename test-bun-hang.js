#!/usr/bin/env node

// Test if asherah still hangs with standard Bun
console.log('Testing asherah-node with Bun...');
console.log('Runtime:', typeof Bun !== 'undefined' ? 'Bun' : 'Node.js');

const asherah = require('./build/Release/asherah.node');
console.log('Module loaded successfully');

const config = {
  ServiceName: 'test-service',
  ProductID: 'test-product',
  KMS: 'static',
  Metastore: 'memory'
};

console.log('\nTesting setup_async...');
const startTime = Date.now();

asherah.setup_async(config).then(() => {
  const elapsed = Date.now() - startTime;
  console.log(`✅ setup_async completed in ${elapsed}ms`);
  process.exit(0);
}).catch((error) => {
  console.error('❌ ERROR:', error.message);
  process.exit(1);
});

// Timeout to detect hang
setTimeout(() => {
  console.error('\n❌ TIMEOUT: setup_async hung after 5 seconds');
  console.error('This confirms the issue still exists with standard Bun');
  process.exit(1);
}, 5000);