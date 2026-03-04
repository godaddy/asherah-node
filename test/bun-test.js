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

async function runTests() {
  let testNum = 0;

  // Test 1: Setup (sync)
  console.log(`${++testNum}. Testing setup (sync)...`);
  asherah.setup(config);
  console.log('   ✅ Setup successful');

  // Test 2: Encrypt (sync)
  console.log(`${++testNum}. Testing encrypt (sync)...`);
  const partitionId = 'bun-test-' + Date.now();
  const testData = 'Bun runtime test: Hello from Bun! 🚀';
  const encrypted = asherah.encrypt(partitionId, Buffer.from(testData, 'utf8'));

  if (!encrypted || encrypted.length === 0) {
    throw new Error('Encryption failed - no data returned');
  }
  console.log(`   ✅ Encrypted ${testData.length} bytes -> ${encrypted.length} chars`);

  // Test 3: Decrypt (sync)
  console.log(`${++testNum}. Testing decrypt (sync)...`);
  const decrypted = asherah.decrypt(partitionId, encrypted);

  if (!decrypted) {
    throw new Error('Decryption failed - no data returned');
  }

  const decryptedText = decrypted.toString('utf8');
  if (decryptedText !== testData) {
    throw new Error(`Decryption mismatch: expected "${testData}", got "${decryptedText}"`);
  }
  console.log('   ✅ Decrypted successfully, data matches');

  // Test 4: Encrypt string (sync)
  console.log(`${++testNum}. Testing encrypt_string (sync)...`);
  const encryptedStr = asherah.encrypt_string(partitionId, testData);

  if (!encryptedStr || encryptedStr.length === 0) {
    throw new Error('String encryption failed - no data returned');
  }
  console.log(`   ✅ Encrypted string ${testData.length} chars -> ${encryptedStr.length} chars`);

  // Test 5: Decrypt string (sync)
  console.log(`${++testNum}. Testing decrypt_string (sync)...`);
  const decryptedStr = asherah.decrypt_string(partitionId, encryptedStr);

  if (decryptedStr !== testData) {
    throw new Error(`String decryption mismatch: expected "${testData}", got "${decryptedStr}"`);
  }
  console.log('   ✅ Decrypted string successfully, data matches');

  // Test 6: Multiple sync cycles
  console.log(`${++testNum}. Testing multiple sync encrypt/decrypt cycles...`);
  for (let i = 0; i < 5; i++) {
    const data = `Test cycle ${i}: ${Math.random()}`;
    const enc = asherah.encrypt(partitionId, Buffer.from(data, 'utf8'));
    const dec = asherah.decrypt(partitionId, enc);
    if (dec.toString('utf8') !== data) {
      throw new Error(`Cycle ${i} failed: data mismatch`);
    }
  }
  console.log('   ✅ 5 sync cycles completed successfully');

  // Test 7: Encrypt async
  console.log(`${++testNum}. Testing encrypt_async...`);
  const encryptedAsync = await asherah.encrypt_async(partitionId, Buffer.from(testData, 'utf8'));

  if (!encryptedAsync || encryptedAsync.length === 0) {
    throw new Error('Async encryption failed - no data returned');
  }
  console.log(`   ✅ Async encrypted ${testData.length} bytes -> ${encryptedAsync.length} chars`);

  // Test 8: Decrypt async
  console.log(`${++testNum}. Testing decrypt_async...`);
  const decryptedAsync = await asherah.decrypt_async(partitionId, encryptedAsync);

  if (!decryptedAsync) {
    throw new Error('Async decryption failed - no data returned');
  }

  const decryptedAsyncText = decryptedAsync.toString('utf8');
  if (decryptedAsyncText !== testData) {
    throw new Error(`Async decryption mismatch: expected "${testData}", got "${decryptedAsyncText}"`);
  }
  console.log('   ✅ Async decrypted successfully, data matches');

  // Test 9: Encrypt string async
  console.log(`${++testNum}. Testing encrypt_string_async...`);
  const encryptedStrAsync = await asherah.encrypt_string_async(partitionId, testData);

  if (!encryptedStrAsync || encryptedStrAsync.length === 0) {
    throw new Error('Async string encryption failed - no data returned');
  }
  console.log(`   ✅ Async encrypted string ${testData.length} chars -> ${encryptedStrAsync.length} chars`);

  // Test 10: Decrypt string async
  console.log(`${++testNum}. Testing decrypt_string_async...`);
  const decryptedStrAsync = await asherah.decrypt_string_async(partitionId, encryptedStrAsync);

  if (decryptedStrAsync !== testData) {
    throw new Error(`Async string decryption mismatch: expected "${testData}", got "${decryptedStrAsync}"`);
  }
  console.log('   ✅ Async decrypted string successfully, data matches');

  // Test 11: Multiple async cycles
  console.log(`${++testNum}. Testing multiple async encrypt/decrypt cycles...`);
  for (let i = 0; i < 5; i++) {
    const data = `Async test cycle ${i}: ${Math.random()}`;
    const enc = await asherah.encrypt_async(partitionId, Buffer.from(data, 'utf8'));
    const dec = await asherah.decrypt_async(partitionId, enc);
    if (dec.toString('utf8') !== data) {
      throw new Error(`Async cycle ${i} failed: data mismatch`);
    }
  }
  console.log('   ✅ 5 async cycles completed successfully');

  // Test 12: Cross sync/async - encrypt sync, decrypt async
  console.log(`${++testNum}. Testing cross sync->async round trip...`);
  const crossEncSync = asherah.encrypt(partitionId, Buffer.from(testData, 'utf8'));
  const crossDecAsync = await asherah.decrypt_async(partitionId, crossEncSync);
  if (crossDecAsync.toString('utf8') !== testData) {
    throw new Error('Cross sync->async round trip failed: data mismatch');
  }
  console.log('   ✅ Sync encrypt -> async decrypt succeeded');

  // Test 13: Cross async/sync - encrypt async, decrypt sync
  console.log(`${++testNum}. Testing cross async->sync round trip...`);
  const crossEncAsync = await asherah.encrypt_async(partitionId, Buffer.from(testData, 'utf8'));
  const crossDecSync = asherah.decrypt(partitionId, crossEncAsync);
  if (crossDecSync.toString('utf8') !== testData) {
    throw new Error('Cross async->sync round trip failed: data mismatch');
  }
  console.log('   ✅ Async encrypt -> sync decrypt succeeded');

  // Test 14: Shutdown (sync)
  console.log(`${++testNum}. Testing shutdown (sync)...`);
  asherah.shutdown();
  console.log('   ✅ Shutdown successful');

  // Test 15: Setup async + shutdown async
  console.log(`${++testNum}. Testing setup_async...`);
  await asherah.setup_async(config);
  console.log('   ✅ Async setup successful');

  console.log(`${++testNum}. Testing encrypt/decrypt after async setup...`);
  const asyncSetupEnc = asherah.encrypt(partitionId, Buffer.from(testData, 'utf8'));
  const asyncSetupDec = asherah.decrypt(partitionId, asyncSetupEnc);
  if (asyncSetupDec.toString('utf8') !== testData) {
    throw new Error('Round trip after async setup failed: data mismatch');
  }
  console.log('   ✅ Round trip after async setup succeeded');

  console.log(`${++testNum}. Testing shutdown_async...`);
  await asherah.shutdown_async();
  console.log('   ✅ Async shutdown successful');

  console.log('');
  console.log(`🎉 All ${testNum} tests PASSED!`);
  console.log('Bun compatibility confirmed ✅');
}

runTests().then(() => {
  process.exit(0);
}).catch((error) => {
  console.error('');
  console.error('❌ Test FAILED:');
  console.error(error.message);
  if (error.stack) {
    console.error('\nStack trace:');
    console.error(error.stack);
  }
  process.exit(1);
});
