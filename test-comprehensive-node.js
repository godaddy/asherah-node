#!/usr/bin/env node

/**
 * Comprehensive Node.js Test Suite for asherah-node
 * 
 * This test suite comprehensively validates that the unified entry point
 * works correctly in Node.js without any regressions.
 */

console.log('=== Comprehensive Node.js Test Suite ===\n');

const crypto = require('crypto');
const path = require('path');

// Track test results
let testsRun = 0;
let testsPassed = 0;

function runTest(testName, testFn) {
  testsRun++;
  console.log(`🧪 Running: ${testName}`);
  
  try {
    testFn();
    testsPassed++;
    console.log(`   ✅ PASSED: ${testName}\n`);
  } catch (error) {
    console.error(`   ❌ FAILED: ${testName}`);
    console.error(`   Error: ${error.message}\n`);
  }
}

// Test 1: Basic module loading
runTest('Basic Module Loading', () => {
  const asherah = require('./src/index.js');
  if (typeof asherah !== 'object') {
    throw new Error('Module did not load as expected object');
  }
  if (typeof asherah.setup !== 'function') {
    throw new Error('setup function not available');
  }
  if (typeof asherah.encrypt !== 'function') {
    throw new Error('encrypt function not available');
  }
  if (typeof asherah.decrypt !== 'function') {
    throw new Error('decrypt function not available');
  }
  if (typeof asherah.shutdown !== 'function') {
    throw new Error('shutdown function not available');
  }
});

// Test 2: Entry point path resolution
runTest('Entry Point Path Resolution', () => {
  // Test that the unified entry point correctly resolves to native module
  const unifiedAsherah = require('./src/index.js');
  const directAsherah = require('./build/Release/asherah.node');
  
  // Should have same function signatures
  if (typeof unifiedAsherah.setup !== typeof directAsherah.setup) {
    throw new Error('Function signature mismatch for setup');
  }
  if (typeof unifiedAsherah.encrypt !== typeof directAsherah.encrypt) {
    throw new Error('Function signature mismatch for encrypt');
  }
});

// Test 3: Complete encryption workflow
runTest('Complete Encryption Workflow', () => {
  const asherah = require('./src/index.js');
  
  // Setup
  asherah.setup({
    ServiceName: 'node-comprehensive-test',
    ProductID: 'comprehensive',
    KMS: 'static',
    Metastore: 'memory',
    Verbose: false
  });
  
  // Test data
  const testData = 'Node.js comprehensive test data 🔐';
  const partition = 'node-test-partition';
  
  // Encrypt
  const encrypted = asherah.encrypt(partition, testData);
  if (!encrypted || typeof encrypted !== 'string') {
    throw new Error('Encryption failed or returned invalid data');
  }
  
  // Decrypt
  const decrypted = asherah.decrypt_string(partition, encrypted);
  if (decrypted !== testData) {
    throw new Error(`Data mismatch: expected "${testData}", got "${decrypted}"`);
  }
  
  // Shutdown
  asherah.shutdown();
});

// Test 4: Multiple partition handling
runTest('Multiple Partition Handling', () => {
  const asherah = require('./src/index.js');
  
  asherah.setup({
    ServiceName: 'node-multipartition-test',
    ProductID: 'multipart',
    KMS: 'static',
    Metastore: 'memory',
    Verbose: false
  });
  
  const partitions = ['partition1', 'partition2', 'partition3'];
  const testData = 'Multi-partition test data';
  
  // Encrypt in multiple partitions
  const encryptedData = {};
  for (const partition of partitions) {
    encryptedData[partition] = asherah.encrypt(partition, testData);
  }
  
  // Decrypt from multiple partitions
  for (const partition of partitions) {
    const decrypted = asherah.decrypt_string(partition, encryptedData[partition]);
    if (decrypted !== testData) {
      throw new Error(`Partition ${partition} data mismatch`);
    }
  }
  
  asherah.shutdown();
});

// Test 5: Buffer handling
runTest('Buffer Handling', () => {
  const asherah = require('./src/index.js');
  
  asherah.setup({
    ServiceName: 'node-buffer-test',
    ProductID: 'buffer',
    KMS: 'static',
    Metastore: 'memory',
    Verbose: false
  });
  
  // Test with Buffer input
  const testBuffer = Buffer.from('Buffer test data 📦', 'utf8');
  const partition = 'buffer-partition';
  
  const encrypted = asherah.encrypt(partition, testBuffer);
  const decrypted = asherah.decrypt(partition, encrypted);
  
  if (!Buffer.isBuffer(decrypted)) {
    throw new Error('Decrypted data is not a Buffer');
  }
  
  if (!testBuffer.equals(decrypted)) {
    throw new Error('Buffer data mismatch');
  }
  
  asherah.shutdown();
});

// Test 6: Large data handling
runTest('Large Data Handling', () => {
  const asherah = require('./src/index.js');
  
  asherah.setup({
    ServiceName: 'node-large-test',
    ProductID: 'large',
    KMS: 'static',
    Metastore: 'memory',
    Verbose: false
  });
  
  // Generate large test data (1MB)
  const largeData = crypto.randomBytes(1024 * 1024).toString('base64');
  const partition = 'large-partition';
  
  const encrypted = asherah.encrypt(partition, largeData);
  const decrypted = asherah.decrypt_string(partition, encrypted);
  
  if (decrypted !== largeData) {
    throw new Error('Large data integrity check failed');
  }
  
  asherah.shutdown();
});

// Test 7: Rapid setup/shutdown cycles
runTest('Rapid Setup/Shutdown Cycles', () => {
  const asherah = require('./src/index.js');
  
  for (let i = 0; i < 5; i++) {
    asherah.setup({
      ServiceName: `node-cycle-test-${i}`,
      ProductID: 'cycle',
      KMS: 'static',
      Metastore: 'memory',
      Verbose: false
    });
    
    const testData = `Cycle test ${i}`;
    const encrypted = asherah.encrypt('cycle-partition', testData);
    const decrypted = asherah.decrypt_string('cycle-partition', encrypted);
    
    if (decrypted !== testData) {
      throw new Error(`Cycle ${i} data mismatch`);
    }
    
    asherah.shutdown();
  }
});

// Test 8: Error handling
runTest('Error Handling', () => {
  const asherah = require('./src/index.js');
  
  // Test encryption without setup (should throw)
  let errorThrown = false;
  try {
    asherah.encrypt('test', 'data');
  } catch (error) {
    errorThrown = true;
  }
  
  if (!errorThrown) {
    throw new Error('Expected error was not thrown for encryption without setup');
  }
});

// Test 9: Environment variable handling
runTest('Environment Variable Handling', () => {
  const originalEnv = process.env.ASHERAH_BUN_VERBOSE;
  
  // Test with verbose enabled
  process.env.ASHERAH_BUN_VERBOSE = '1';
  delete require.cache[path.resolve('./src/index.js')];
  const asherahVerbose = require('./src/index.js');
  
  // Test with verbose disabled
  process.env.ASHERAH_BUN_VERBOSE = '0';
  delete require.cache[path.resolve('./src/index.js')];
  const asherahQuiet = require('./src/index.js');
  
  // Restore original environment
  if (originalEnv !== undefined) {
    process.env.ASHERAH_BUN_VERBOSE = originalEnv;
  } else {
    delete process.env.ASHERAH_BUN_VERBOSE;
  }
  
  // Both should work identically
  if (typeof asherahVerbose.setup !== 'function' || typeof asherahQuiet.setup !== 'function') {
    throw new Error('Environment variable handling affected module loading');
  }
});

// Test 10: Memory management
runTest('Memory Management', () => {
  const asherah = require('./src/index.js');
  
  asherah.setup({
    ServiceName: 'node-memory-test',
    ProductID: 'memory',
    KMS: 'static',
    Metastore: 'memory',
    Verbose: false
  });
  
  // Force garbage collection if available
  if (global.gc) {
    global.gc();
  }
  
  const initialMemory = process.memoryUsage();
  
  // Perform many encryption/decryption operations
  for (let i = 0; i < 100; i++) {
    const data = `Memory test iteration ${i}`;
    const encrypted = asherah.encrypt('memory-partition', data);
    const decrypted = asherah.decrypt_string('memory-partition', encrypted);
    
    if (decrypted !== data) {
      throw new Error(`Memory test iteration ${i} failed`);
    }
  }
  
  if (global.gc) {
    global.gc();
  }
  
  const finalMemory = process.memoryUsage();
  
  // Check for reasonable memory usage (not a strict test, just sanity check)
  const memoryIncrease = finalMemory.heapUsed - initialMemory.heapUsed;
  if (memoryIncrease > 50 * 1024 * 1024) { // 50MB threshold
    console.warn(`   ⚠️  High memory increase detected: ${Math.round(memoryIncrease / 1024 / 1024)}MB`);
  }
  
  asherah.shutdown();
});

// Summary
console.log('=== Test Summary ===');
console.log(`Tests Run: ${testsRun}`);
console.log(`Tests Passed: ${testsPassed}`);
console.log(`Tests Failed: ${testsRun - testsPassed}`);

if (testsPassed === testsRun) {
  console.log('\n🎉 ALL TESTS PASSED! Node.js integration is working correctly.');
  process.exit(0);
} else {
  console.log('\n❌ SOME TESTS FAILED! Please review the failures above.');
  process.exit(1);
}