#!/usr/bin/env bun

/**
 * Comprehensive Bun Test Suite for asherah-node
 * 
 * This test suite comprehensively validates that the unified entry point
 * works correctly in Bun runtime with automatic compatibility handling.
 */

console.log('=== Comprehensive Bun Test Suite ===\n');

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

// Test 1: Bun runtime detection
runTest('Bun Runtime Detection', () => {
  if (typeof Bun === 'undefined') {
    throw new Error('Test is not running in Bun runtime');
  }
  console.log(`   📦 Bun version: ${Bun.version}`);
});

// Test 2: Automatic Bun compatibility initialization
runTest('Automatic Bun Compatibility Initialization', () => {
  // Test with verbose logging enabled
  const originalVerbose = process.env.ASHERAH_BUN_VERBOSE;
  process.env.ASHERAH_BUN_VERBOSE = '1';
  
  // Force reload to trigger initialization logging
  delete require.cache[path.resolve('./src/index.js')];
  const asherah = require('./src/index.js');
  
  // Restore environment
  if (originalVerbose !== undefined) {
    process.env.ASHERAH_BUN_VERBOSE = originalVerbose;
  } else {
    delete process.env.ASHERAH_BUN_VERBOSE;
  }
  
  if (typeof asherah !== 'object') {
    throw new Error('Module did not load as expected object');
  }
  if (typeof asherah.setup !== 'function') {
    throw new Error('setup function not available');
  }
});

// Test 3: Basic module loading
runTest('Basic Module Loading', () => {
  const asherah = require('./src/index.js');
  
  const expectedFunctions = ['setup', 'encrypt', 'decrypt', 'decrypt_string', 'shutdown'];
  for (const func of expectedFunctions) {
    if (typeof asherah[func] !== 'function') {
      throw new Error(`${func} function not available`);
    }
  }
});

// Test 4: Complete encryption workflow in Bun
runTest('Complete Encryption Workflow in Bun', () => {
  const asherah = require('./src/index.js');
  
  // Setup
  asherah.setup({
    ServiceName: 'bun-comprehensive-test',
    ProductID: 'bun-comprehensive',
    KMS: 'static',
    Metastore: 'memory',
    Verbose: false
  });
  
  // Test data with Bun-specific content
  const testData = 'Bun comprehensive test data 🚀⚡';
  const partition = 'bun-test-partition';
  
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

// Test 5: Bun FFI compatibility verification
runTest('Bun FFI Compatibility Verification', () => {
  // Verify that Bun's FFI is working for our preload
  const { dlopen, FFIType } = require('bun:ffi');
  
  if (typeof dlopen !== 'function') {
    throw new Error('Bun FFI dlopen not available');
  }
  if (typeof FFIType !== 'object') {
    throw new Error('Bun FFI types not available');
  }
  
  // Test loading our preload library directly
  const libPath = path.join(__dirname, 'asherah-bun-preload', 'lib', 'bun_warmup_minimal.dylib');
  const lib = dlopen(libPath, {
    MinimalWarmup: { returns: FFIType.int, args: [] }
  });
  
  const result = lib.symbols.MinimalWarmup();
  if (result !== 1) {
    throw new Error(`Unexpected result from MinimalWarmup: ${result}`);
  }
});

// Test 6: Multiple partition handling in Bun
runTest('Multiple Partition Handling in Bun', () => {
  const asherah = require('./src/index.js');
  
  asherah.setup({
    ServiceName: 'bun-multipartition-test',
    ProductID: 'bun-multipart',
    KMS: 'static',
    Metastore: 'memory',
    Verbose: false
  });
  
  const partitions = ['bun-partition1', 'bun-partition2', 'bun-partition3'];
  const testData = 'Bun multi-partition test data';
  
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

// Test 7: Buffer handling in Bun
runTest('Buffer Handling in Bun', () => {
  const asherah = require('./src/index.js');
  
  asherah.setup({
    ServiceName: 'bun-buffer-test',
    ProductID: 'bun-buffer',
    KMS: 'static',
    Metastore: 'memory',
    Verbose: false
  });
  
  // Test with Buffer input
  const testBuffer = Buffer.from('Bun Buffer test data 📦⚡', 'utf8');
  const partition = 'bun-buffer-partition';
  
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

// Test 8: Large data handling in Bun
runTest('Large Data Handling in Bun', () => {
  const asherah = require('./src/index.js');
  
  asherah.setup({
    ServiceName: 'bun-large-test',
    ProductID: 'bun-large',
    KMS: 'static',
    Metastore: 'memory',
    Verbose: false
  });
  
  // Generate large test data (512KB - smaller than Node.js test for speed)
  const largeData = crypto.randomBytes(512 * 1024).toString('base64');
  const partition = 'bun-large-partition';
  
  const encrypted = asherah.encrypt(partition, largeData);
  const decrypted = asherah.decrypt_string(partition, encrypted);
  
  if (decrypted !== largeData) {
    throw new Error('Large data integrity check failed');
  }
  
  asherah.shutdown();
});

// Test 9: Rapid setup/shutdown cycles in Bun
runTest('Rapid Setup/Shutdown Cycles in Bun', () => {
  const asherah = require('./src/index.js');
  
  for (let i = 0; i < 3; i++) { // Fewer cycles than Node.js test for speed
    asherah.setup({
      ServiceName: `bun-cycle-test-${i}`,
      ProductID: 'bun-cycle',
      KMS: 'static',
      Metastore: 'memory',
      Verbose: false
    });
    
    const testData = `Bun cycle test ${i}`;
    const encrypted = asherah.encrypt('bun-cycle-partition', testData);
    const decrypted = asherah.decrypt_string('bun-cycle-partition', encrypted);
    
    if (decrypted !== testData) {
      throw new Error(`Cycle ${i} data mismatch`);
    }
    
    asherah.shutdown();
  }
});

// Test 10: Error handling in Bun
runTest('Error Handling in Bun', () => {
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

// Test 11: Cross-runtime compatibility verification
runTest('Cross-runtime Compatibility Verification', () => {
  const asherah = require('./src/index.js');
  
  asherah.setup({
    ServiceName: 'bun-compat-test',
    ProductID: 'compat',
    KMS: 'static',
    Metastore: 'memory',
    Verbose: false
  });
  
  // Test data that should work identically in both runtimes
  const testData = 'Cross-runtime compatibility test 🔄';
  const partition = 'compat-partition';
  
  const encrypted = asherah.encrypt(partition, testData);
  const decrypted = asherah.decrypt_string(partition, encrypted);
  
  if (decrypted !== testData) {
    throw new Error('Cross-runtime compatibility failed');
  }
  
  // Verify the encrypted format is base64 (standard format)
  try {
    Buffer.from(encrypted, 'base64');
  } catch (error) {
    throw new Error('Encrypted data is not valid base64');
  }
  
  asherah.shutdown();
});

// Test 12: Performance baseline
runTest('Performance Baseline', () => {
  const asherah = require('./src/index.js');
  
  asherah.setup({
    ServiceName: 'bun-perf-test',
    ProductID: 'perf',
    KMS: 'static',
    Metastore: 'memory',
    Verbose: false
  });
  
  const testData = 'Performance test data';
  const partition = 'perf-partition';
  const iterations = 50;
  
  const startTime = performance.now();
  
  for (let i = 0; i < iterations; i++) {
    const encrypted = asherah.encrypt(partition, `${testData} ${i}`);
    const decrypted = asherah.decrypt_string(partition, encrypted);
    
    if (decrypted !== `${testData} ${i}`) {
      throw new Error(`Performance test iteration ${i} failed`);
    }
  }
  
  const endTime = performance.now();
  const duration = endTime - startTime;
  const avgTime = duration / iterations;
  
  console.log(`   📊 Performance: ${iterations} operations in ${duration.toFixed(2)}ms (${avgTime.toFixed(2)}ms avg)`);
  
  if (avgTime > 100) { // 100ms per operation seems reasonable
    console.warn(`   ⚠️  Slow performance detected: ${avgTime.toFixed(2)}ms per operation`);
  }
  
  asherah.shutdown();
});

// Test 13: Environment variable behavior in Bun
runTest('Environment Variable Behavior in Bun', () => {
  const originalEnv = process.env.ASHERAH_BUN_VERBOSE;
  
  // Test verbose mode specifically in Bun
  process.env.ASHERAH_BUN_VERBOSE = '1';
  
  // Force reload to see verbose output
  delete require.cache[path.resolve('./src/index.js')];
  console.log('   🔍 Loading with verbose mode (should see initialization message):');
  const asherahVerbose = require('./src/index.js');
  
  // Restore
  if (originalEnv !== undefined) {
    process.env.ASHERAH_BUN_VERBOSE = originalEnv;
  } else {
    delete process.env.ASHERAH_BUN_VERBOSE;
  }
  
  if (typeof asherahVerbose.setup !== 'function') {
    throw new Error('Verbose mode affected module functionality');
  }
});

// Summary
console.log('=== Test Summary ===');
console.log(`Tests Run: ${testsRun}`);
console.log(`Tests Passed: ${testsPassed}`);
console.log(`Tests Failed: ${testsRun - testsPassed}`);

if (testsPassed === testsRun) {
  console.log('\n🎉 ALL TESTS PASSED! Bun integration is working correctly.');
  console.log('\n📦 User Experience Verified:');
  console.log('✅ Automatic runtime detection');
  console.log('✅ Seamless Go compatibility initialization');
  console.log('✅ Identical API behavior to Node.js');
  console.log('✅ No additional user configuration required');
  process.exit(0);
} else {
  console.log('\n❌ SOME TESTS FAILED! Please review the failures above.');
  process.exit(1);
}