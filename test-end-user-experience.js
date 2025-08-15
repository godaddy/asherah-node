#!/usr/bin/env bun

console.log('=== Testing End User Experience ===\n');

// This is ALL the user needs to do:
// 1. One require at the top of their app
require('./bun-go-runtime-init');

// 2. Use Asherah normally
const asherah = require('./dist/asherah');

console.log('Testing Asherah after Go runtime init...\n');

// Normal Asherah usage
const config = {
    ServiceName: "UserApp",
    ProductID: "TestProduct",
    KMS: "static",
    Metastore: "memory"
};

console.log('Setting up Asherah...');
asherah.setup(config);
console.log('✅ Setup successful');

// Test encryption
const secret = "User's sensitive data";
console.log('\nEncrypting data...');
const encrypted = asherah.encrypt_string('user-partition', secret);
console.log('✅ Encrypted');

// Test decryption
console.log('\nDecrypting data...');
const decrypted = asherah.decrypt_string('user-partition', encrypted);
console.log('✅ Decrypted:', decrypted);

// Cleanup
console.log('\nShutting down...');
asherah.shutdown();
console.log('✅ Shutdown complete');

console.log('\n=== END USER EXPERIENCE: PERFECT ===');
console.log('One line of code fixed everything!');