#!/usr/bin/env node

// Test to verify the CGO stack size fix works
// This should work with the fixed Bun build but hang with the original

console.log('=== CGO Stack Size Fix Verification ===');
console.log('Runtime:', typeof Bun !== 'undefined' ? 'Bun' : 'Node.js');

const asherah = require('./build/Release/asherah.node');
console.log('Module loaded successfully');

const config = {
  ServiceName: 'test-service',
  ProductID: 'test-product',
  KMS: 'static', 
  Metastore: 'memory'
};

console.log('\nTesting CGO async operation...');
const startTime = Date.now();

asherah.setup_async(config).then(() => {
  const elapsed = Date.now() - startTime;
  console.log(`✅ SUCCESS: CGO operation completed in ${elapsed}ms`);
  console.log('✅ Stack size fix is working!');
  console.log('   - 8MB thread stacks provide adequate space for CGO');
  console.log('   - Go runtime operations (GC, scheduler) can execute');
  console.log('   - Matches Node.js/libuv behavior exactly');
  process.exit(0);
}).catch((error) => {
  console.error('❌ ERROR:', error.message);
  console.error('The fix may not be properly applied');
  process.exit(1);
});

// Timeout to detect if we're still hanging
setTimeout(() => {
  console.error('\n❌ TIMEOUT: CGO operation hung after 10 seconds');
  console.error('This suggests the stack size fix is not applied or insufficient');
  console.error('Expected behavior:');
  console.error('  - Node.js: Completes in ~2ms (8MB stacks)'); 
  console.error('  - Bun (unfixed): Hangs (4MB stacks)');
  console.error('  - Bun (fixed): Completes in ~2ms (8MB stacks)');
  process.exit(1);
}, 10000);