console.log('Testing simplest async function...');

const asherah = require('./build/Release/asherah.node');
console.log('Module loaded');

console.log('Available functions:', Object.keys(asherah));

// Try the simplest async function
(async () => {
  try {
    console.log('Calling asherah.setup (async)...');
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
    const result = await asherah.setup(config);
    console.log('✅ Setup succeeded:', result);
  } catch (e) {
    console.error('❌ Setup failed:', e.message);
  }
})();