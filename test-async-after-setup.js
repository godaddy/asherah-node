// Test if sync works after async initializes Go runtime
console.log('Testing: Initialize Go runtime with setenv, then encrypt');

const asherah = require('./build/Release/asherah.node');
console.log('Module loaded');

// First, initialize Go runtime with a simple function that should work
console.log('\n1. Calling setenv to initialize Go runtime...');
try {
  asherah.setenv('{}');
  console.log('✅ Setenv succeeded - Go runtime initialized!');
  
  // Now try encrypt which requires full setup
  console.log('\n2. Now calling setup...');
  const config = JSON.stringify({
    "ServiceName": "TestService",
    "ProductID": "TestProduct", 
    "KMS": "static",
    "Metastore": "memory",
    "Verbose": true,
    "StaticKeyManagement": {
      "KEY_ENCRYPTION_KEY": "thisIsAStaticMasterKeyForTesting"
    }
  });
  
  asherah.setupSync(config);
  console.log('✅ Setup succeeded!');
  
  console.log('\n3. Calling encrypt...');
  const encrypted = asherah.encrypt('TestPartition', Buffer.from('test data'));
  console.log('✅ Encrypt succeeded!');
  
} catch (e) {
  console.error('❌ Error:', e.message);
}