#!/usr/bin/env bun

console.log('=== Testing Unified Bun Integration ===\n');
console.log('This tests the built-in Bun support with automatic runtime detection.\n');

// Test the unified entry point (should auto-detect Bun and initialize compatibility)
console.log('Loading asherah-node with unified entry point...');
const asherah = require('./src/index.js');
console.log('✅ asherah-node loaded with automatic Bun compatibility\n');

// Complete workflow test
console.log('Testing complete workflow...');

try {
    // Setup
    console.log('   Setting up...');
    asherah.setup({
        ServiceName: 'unified-bun-test',
        ProductID: 'unified-test',
        KMS: 'static',
        Metastore: 'memory',
        Verbose: false
    });
    console.log('   ✅ Setup successful');

    // Test operations
    const testData = 'Unified Bun integration test data 🚀';
    const partition = 'unified-test-partition';
    
    console.log('   Encrypting data...');
    const encrypted = asherah.encrypt(partition, testData);
    console.log('   ✅ Encryption successful');
    
    console.log('   Decrypting data...');
    const decrypted = asherah.decrypt_string(partition, encrypted);
    console.log('   ✅ Decryption successful');
    
    // Verify
    if (decrypted !== testData) {
        throw new Error('Data mismatch');
    }
    console.log('   ✅ Data verification passed');

    // Shutdown
    console.log('   Shutting down...');
    asherah.shutdown();
    console.log('   ✅ Shutdown successful');

    console.log('\n🎉 SUCCESS: Unified Bun integration works perfectly!');
    console.log('\n📦 User Experience:');
    console.log('npm install asherah-node  # No additional packages needed!');
    console.log("const asherah = require('asherah-node'); // Auto-detects Bun");
    console.log('// Use normally - works in both Node.js and Bun');

} catch (error) {
    console.error('\n❌ Unified integration test failed:', error);
    process.exit(1);
}