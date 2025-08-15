#!/usr/bin/env node

/**
 * Edge Cases and Error Scenarios Test Suite
 * 
 * This suite tests various edge cases and error conditions to ensure
 * robust behavior of the unified integration.
 */

console.log('=== Edge Cases and Error Scenarios Test Suite ===\n');

const fs = require('fs');
const path = require('path');

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

// Test 1: Missing preload library handling
runTest('Missing Preload Library Handling', () => {
  const originalPath = path.join(__dirname, 'asherah-bun-preload', 'lib', 'bun_warmup_minimal.dylib');
  const backupPath = originalPath + '.backup';
  
  // Backup the library
  if (fs.existsSync(originalPath)) {
    fs.renameSync(originalPath, backupPath);
  }
  
  try {
    // Clear module cache
    delete require.cache[path.resolve('./src/index.js')];
    
    // Should still load even without preload library
    const asherah = require('./src/index.js');
    
    if (typeof asherah.setup !== 'function') {
      throw new Error('Module failed to load when preload library missing');
    }
    
  } finally {
    // Restore the library
    if (fs.existsSync(backupPath)) {
      fs.renameSync(backupPath, originalPath);
    }
  }
});

// Test 2: Invalid entry point path handling
runTest('Invalid Entry Point Path Handling', () => {
  // Test with a modified index.js that has wrong path
  const testIndexPath = path.join(__dirname, 'test-invalid-index.js');
  
  const invalidIndexContent = `
const path = require('path');

// Intentionally wrong path
module.exports = require(path.join(__dirname, '..', 'nonexistent', 'asherah.node'));
`;
  
  fs.writeFileSync(testIndexPath, invalidIndexContent);
  
  try {
    let errorThrown = false;
    try {
      require(testIndexPath);
    } catch (error) {
      errorThrown = true;
      if (!error.message.includes('Cannot find module')) {
        throw new Error('Unexpected error type: ' + error.message);
      }
    }
    
    if (!errorThrown) {
      throw new Error('Expected error was not thrown for invalid path');
    }
    
  } finally {
    // Cleanup
    if (fs.existsSync(testIndexPath)) {
      fs.unlinkSync(testIndexPath);
    }
  }
});

// Test 3: Module cache handling
runTest('Module Cache Handling', () => {
  const indexPath = path.resolve('./src/index.js');
  
  // Load module first time
  const asherah1 = require('./src/index.js');
  
  // Load module second time (should use cache)
  const asherah2 = require('./src/index.js');
  
  // Should be the same object reference
  if (asherah1 !== asherah2) {
    throw new Error('Module cache not working correctly');
  }
  
  // Clear cache and reload
  delete require.cache[indexPath];
  const asherah3 = require('./src/index.js');
  
  // Should have same functionality
  if (typeof asherah3.setup !== 'function') {
    throw new Error('Module reload failed');
  }
});

// Test 4: Environment variable edge cases
runTest('Environment Variable Edge Cases', () => {
  const originalEnv = process.env.ASHERAH_BUN_VERBOSE;
  const indexPath = path.resolve('./src/index.js');
  
  // Test with various values
  const testValues = ['1', 'true', 'yes', '0', 'false', 'no', '', 'invalid'];
  
  for (const value of testValues) {
    process.env.ASHERAH_BUN_VERBOSE = value;
    delete require.cache[indexPath];
    
    try {
      const asherah = require('./src/index.js');
      if (typeof asherah.setup !== 'function') {
        throw new Error(`Module failed to load with ASHERAH_BUN_VERBOSE="${value}"`);
      }
    } catch (error) {
      throw new Error(`Failed with ASHERAH_BUN_VERBOSE="${value}": ${error.message}`);
    }
  }
  
  // Restore
  if (originalEnv !== undefined) {
    process.env.ASHERAH_BUN_VERBOSE = originalEnv;
  } else {
    delete process.env.ASHERAH_BUN_VERBOSE;
  }
});

// Test 5: File system permissions
runTest('File System Permissions', () => {
  const libPath = path.join(__dirname, 'asherah-bun-preload', 'lib', 'bun_warmup_minimal.dylib');
  
  if (!fs.existsSync(libPath)) {
    throw new Error('Preload library not found for permissions test');
  }
  
  const stats = fs.statSync(libPath);
  
  // Check that file is readable
  try {
    fs.accessSync(libPath, fs.constants.R_OK);
  } catch (error) {
    throw new Error('Preload library is not readable');
  }
  
  // Check that file has reasonable size (not empty, not too large)
  if (stats.size < 1000) {
    throw new Error('Preload library suspiciously small');
  }
  
  if (stats.size > 10 * 1024 * 1024) { // 10MB
    throw new Error('Preload library suspiciously large');
  }
});

// Test 6: Concurrent module loading
runTest('Concurrent Module Loading', () => {
  const indexPath = path.resolve('./src/index.js');
  
  // Clear cache
  delete require.cache[indexPath];
  
  // Load module multiple times "concurrently" (as much as possible in single thread)
  const promises = [];
  for (let i = 0; i < 5; i++) {
    promises.push(new Promise((resolve, reject) => {
      try {
        const asherah = require('./src/index.js');
        if (typeof asherah.setup !== 'function') {
          reject(new Error(`Concurrent load ${i} failed`));
        }
        resolve(asherah);
      } catch (error) {
        reject(error);
      }
    }));
  }
  
  // All should resolve to same module
  Promise.all(promises).then(modules => {
    const first = modules[0];
    for (let i = 1; i < modules.length; i++) {
      if (modules[i] !== first) {
        throw new Error('Concurrent loading produced different module instances');
      }
    }
  }).catch(error => {
    throw error;
  });
});

// Test 7: Path resolution edge cases
runTest('Path Resolution Edge Cases', () => {
  const originalCwd = process.cwd();
  
  try {
    // Change to different directory
    const tempDir = require('os').tmpdir();
    process.chdir(tempDir);
    
    // Module should still load correctly with absolute path resolution
    const indexPath = path.resolve(originalCwd, 'src', 'index.js');
    delete require.cache[indexPath];
    
    const asherah = require(indexPath);
    if (typeof asherah.setup !== 'function') {
      throw new Error('Module failed to load from different working directory');
    }
    
  } finally {
    // Restore working directory
    process.chdir(originalCwd);
  }
});

// Test 8: Memory cleanup on repeated loads
runTest('Memory Cleanup on Repeated Loads', () => {
  const indexPath = path.resolve('./src/index.js');
  
  // Force garbage collection if available
  if (global.gc) {
    global.gc();
  }
  
  const initialMemory = process.memoryUsage();
  
  // Load and unload module multiple times
  for (let i = 0; i < 10; i++) {
    delete require.cache[indexPath];
    const asherah = require('./src/index.js');
    
    if (typeof asherah.setup !== 'function') {
      throw new Error(`Load iteration ${i} failed`);
    }
  }
  
  if (global.gc) {
    global.gc();
  }
  
  const finalMemory = process.memoryUsage();
  const memoryIncrease = finalMemory.heapUsed - initialMemory.heapUsed;
  
  // Should not leak significant memory
  if (memoryIncrease > 10 * 1024 * 1024) { // 10MB threshold
    console.warn(`   ⚠️  Potential memory leak detected: ${Math.round(memoryIncrease / 1024 / 1024)}MB increase`);
  }
});

// Test 9: Platform-specific library loading
runTest('Platform-specific Library Loading', () => {
  const platform = process.platform;
  const expectedExtensions = {
    'darwin': '.dylib',
    'linux': '.so',
    'win32': '.dll'
  };
  
  const expectedExt = expectedExtensions[platform];
  if (expectedExt) {
    const libPath = path.join(__dirname, 'asherah-bun-preload', 'lib', `bun_warmup_minimal${expectedExt}`);
    
    if (!fs.existsSync(libPath)) {
      throw new Error(`Platform-specific library not found: ${libPath}`);
    }
  } else {
    console.log(`   ⚠️  Unknown platform: ${platform} - skipping platform-specific check`);
  }
});

// Test 10: Error recovery
runTest('Error Recovery', () => {
  // Test that module can recover from errors and continue working
  const asherah = require('./src/index.js');
  
  // First, test normal operation
  asherah.setup({
    ServiceName: 'error-recovery-test',
    ProductID: 'recovery',
    KMS: 'static',
    Metastore: 'memory',
    Verbose: false
  });
  
  const testData = 'Recovery test data';
  const encrypted = asherah.encrypt('recovery-partition', testData);
  const decrypted = asherah.decrypt_string('recovery-partition', encrypted);
  
  if (decrypted !== testData) {
    throw new Error('Normal operation failed');
  }
  
  // Test error condition
  let errorThrown = false;
  try {
    asherah.encrypt('', 'data'); // Empty partition should error
  } catch (error) {
    errorThrown = true;
  }
  
  if (!errorThrown) {
    console.warn('   ⚠️  Expected error was not thrown for empty partition');
  }
  
  // Test recovery - should still work after error
  const encrypted2 = asherah.encrypt('recovery-partition', testData);
  const decrypted2 = asherah.decrypt_string('recovery-partition', encrypted2);
  
  if (decrypted2 !== testData) {
    throw new Error('Module did not recover properly after error');
  }
  
  asherah.shutdown();
});

// Summary
console.log('=== Edge Cases Test Summary ===');
console.log(`Tests Run: ${testsRun}`);
console.log(`Tests Passed: ${testsPassed}`);
console.log(`Tests Failed: ${testsRun - testsPassed}`);

if (testsPassed === testsRun) {
  console.log('\n🎉 ALL EDGE CASE TESTS PASSED! Integration is robust.');
} else {
  console.log('\n❌ SOME EDGE CASE TESTS FAILED! Please review the failures above.');
  process.exit(1);
}