console.log('Testing async setup with proper config...');

const asherah = require('./build/Release/asherah.node');
console.log('Module loaded');

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

console.log('Calling setup_async (the actual async version)...');
try {
  const promise = asherah.setup_async(config);
  console.log('setup_async returned:', promise);
  console.log('Is promise?', promise && typeof promise.then === 'function');
  
  if (promise && typeof promise.then === 'function') {
    promise.then(result => {
      console.log('✅ Async setup succeeded:', result);
      
      // Now try sync after async
      console.log('\n Testing sync encrypt after async setup...');
      try {
        const encrypted = asherah.encrypt('TestPartition', Buffer.from('test'));
        console.log('✅ Sync encrypt worked!', encrypted);
      } catch (e) {
        console.error('❌ Sync encrypt failed:', e.message);
      }
    }).catch(err => {
      console.error('❌ Async setup failed:', err.message);
    });
  }
} catch (e) {
  console.error('❌ setup_async threw:', e.message);
}