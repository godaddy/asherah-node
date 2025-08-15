// Test if thread pool is working at all
console.log('Testing basic thread pool operation...');

// Use crypto.randomBytes which should use thread pool
const crypto = require('crypto');

console.log('1. Testing async crypto (uses thread pool)...');
crypto.randomBytes(16, (err, buf) => {
  if (err) {
    console.error('❌ Crypto failed:', err.message);
  } else {
    console.log('✅ Crypto succeeded, buffer length:', buf.length);
  }
  
  // Now test native module
  console.log('\n2. Testing native module async...');
  const asherah = require('./build/Release/asherah.node');
  console.log('Module loaded');
  
  // The simplest async test - just the setup
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
  
  asherah.setup(config)
    .then(result => {
      console.log('✅ Native async succeeded:', result);
    })
    .catch(err => {
      console.error('❌ Native async failed:', err.message);
    });
});