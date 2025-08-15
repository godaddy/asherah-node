console.log('Testing: async setup -> sync encrypt workflow');

const asherah = require('./build/Release/asherah.node');
console.log('Module loaded');

async function test() {
  // 1. First try async setup (should work with our fixes)
  console.log('\n1. Calling setup (async)...');
  try {
    const config = JSON.stringify({
      "KMS": "static",
      "Metastore": "memory", 
      "ServiceName": "TestService",
      "ProductID": "TestProduct",
      "Verbose": true,
      "ExpireAfter": 60,
      "CheckInterval": 30,
      "StaticKeyManagement": {
        "KEY_ENCRYPTION_KEY": "thisIsAStaticMasterKeyForTesting"
      }
    });
    
    const setupResult = await asherah.setup(config);
    console.log('✅ Async setup succeeded, result:', setupResult);
  } catch (e) {
    console.error('❌ Async setup failed:', e.message);
    return;
  }

  // 2. Now try sync encrypt - this should work if Go runtime is already initialized
  console.log('\n2. Calling encrypt (sync) after async setup...');
  try {
    const partitionId = JSON.stringify({ partition: "TestPartition" });
    const data = JSON.stringify({ data: Buffer.from("test data").toString('base64') });
    
    const encrypted = asherah.encrypt(partitionId, data);
    console.log('✅ SYNC ENCRYPT SUCCEEDED! Result:', encrypted);
    
    // 3. Try sync decrypt
    console.log('\n3. Calling decrypt (sync)...');
    const decrypted = asherah.decrypt(partitionId, encrypted);
    console.log('✅ SYNC DECRYPT SUCCEEDED! Result:', decrypted);
    
  } catch (e) {
    console.error('❌ Sync operation failed:', e.message);
    console.error('Stack:', e.stack);
  }

  // 4. Try shutting down
  console.log('\n4. Calling shutdown (async)...');
  try {
    await asherah.shutdown();
    console.log('✅ Shutdown succeeded');
  } catch (e) {
    console.error('❌ Shutdown failed:', e.message);
  }
}

test().catch(console.error);