#!/usr/bin/env bun

console.log('Testing Asherah with Bun wrapper...\n');

try {
    // Load Asherah through the wrapper
    const asherah = require('./asherah-bun-wrapper');
    
    console.log('Asherah exports:', Object.keys(asherah));
    
    // Test setup
    const config = {
        ServiceName: "test",
        ProductID: "test",
        KMS: "static",
        Metastore: "memory"
    };
    
    console.log('\nCalling setup...');
    asherah.setup(config);
    console.log('✅ Setup succeeded!');
    
    // Test encryption
    const data = Buffer.from('Hello, Asherah!');
    console.log('\nTesting encryption...');
    const encrypted = asherah.encrypt('test-partition', data);
    console.log('✅ Encrypted:', encrypted.substring(0, 50) + '...');
    
    // Test decryption
    console.log('\nTesting decryption...');
    const decrypted = asherah.decrypt('test-partition', encrypted);
    console.log('✅ Decrypted:', decrypted.toString());
    
    // Shutdown
    console.log('\nShutting down...');
    asherah.shutdown();
    console.log('✅ Shutdown complete');
    
    console.log('\n=== ALL TESTS PASSED ===');
    
} catch (e) {
    console.error('❌ Test failed:', e.message);
    process.exit(1);
}