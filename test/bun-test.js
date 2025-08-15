#!/usr/bin/env bun

/**
 * Simple Bun test runner for CI/CD integration
 * 
 * This script runs the Bun compatibility test and exits with
 * appropriate status codes for CI/CD pipelines.
 * 
 * Usage: bun test/bun-test.js
 */

const asherah = require('../src/index');

console.log('=== Bun Compatibility Test ===');
console.log(`Runtime: ${typeof Bun !== 'undefined' ? 'Bun' : 'Not Bun'}`);
console.log(`Platform: ${process.platform} ${process.arch}`);
console.log('');

const config = {
  KMS: 'static',
  Metastore: 'memory',
  ServiceName: 'bun-test',
  ProductID: 'bun-test',
  Verbose: false,
  EnableSessionCaching: false,
  ExpireAfter: 60,
  CheckInterval: 1
};

let success = true;

try {
  // Test 1: Setup
  console.log('1. Testing setup...');
  asherah.setup(config);
  console.log('   ✅ Setup successful');
  
  // Test 2: Encrypt
  console.log('2. Testing encryption...');
  const partitionId = 'bun-test-' + Date.now();
  const testData = 'Bun runtime test: Hello from Bun! 🚀';
  const encrypted = asherah.encrypt(partitionId, Buffer.from(testData, 'utf8'));
  
  if (!encrypted || encrypted.length === 0) {
    throw new Error('Encryption failed - no data returned');
  }
  console.log(`   ✅ Encrypted ${testData.length} bytes -> ${encrypted.length} chars`);
  
  // Test 3: Decrypt
  console.log('3. Testing decryption...');
  const decrypted = asherah.decrypt(partitionId, encrypted);
  
  if (!decrypted) {
    throw new Error('Decryption failed - no data returned');
  }
  
  const decryptedText = decrypted.toString('utf8');
  if (decryptedText !== testData) {
    throw new Error(`Decryption mismatch: expected "${testData}", got "${decryptedText}"`);
  }
  console.log('   ✅ Decrypted successfully, data matches');
  
  // Test 4: Multiple cycles
  console.log('4. Testing multiple encrypt/decrypt cycles...');
  for (let i = 0; i < 5; i++) {
    const data = `Test cycle ${i}: ${Math.random()}`;
    const enc = asherah.encrypt(partitionId, Buffer.from(data, 'utf8'));
    const dec = asherah.decrypt(partitionId, enc);
    if (dec.toString('utf8') !== data) {
      throw new Error(`Cycle ${i} failed: data mismatch`);
    }
  }
  console.log('   ✅ 5 cycles completed successfully');
  
  // Test 5: Shutdown
  console.log('5. Testing shutdown...');
  asherah.shutdown();
  console.log('   ✅ Shutdown successful');
  
  console.log('');
  console.log('🎉 All tests PASSED!');
  console.log('Bun compatibility confirmed ✅');
  
} catch (error) {
  console.error('');
  console.error('❌ Test FAILED:');
  console.error(error.message);
  if (error.stack) {
    console.error('\nStack trace:');
    console.error(error.stack);
  }
  success = false;
}

// Exit with appropriate code
process.exit(success ? 0 : 1);
