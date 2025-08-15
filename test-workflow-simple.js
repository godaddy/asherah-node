console.log('Testing full workflow...');

const asherah = require('./build/Release/asherah.node');
console.log('Module loaded');

// Create an instance
const instance = new asherah.Asherah();
console.log('Instance created');

async function test() {
  // 1. Setup async (should work based on our fixes)
  console.log('\n1. Calling setupAsync...');
  try {
    await instance.setupAsync(JSON.stringify({
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
    }));
    console.log('✅ setupAsync succeeded');
  } catch (e) {
    console.error('❌ setupAsync failed:', e.message);
    return;
  }

  // 2. Try sync encrypt after async setup
  console.log('\n2. Calling encryptSync after async setup...');
  try {
    const encrypted = instance.encryptSync('TestPartition', Buffer.from('test data'));
    console.log('✅ encryptSync succeeded! Result length:', encrypted.length);
    
    // 3. Try sync decrypt
    console.log('\n3. Calling decryptSync...');
    const decrypted = instance.decryptSync('TestPartition', encrypted);
    console.log('✅ decryptSync succeeded! Result:', decrypted.toString());
  } catch (e) {
    console.error('❌ Sync operation failed:', e.message);
  }

  // 4. Try async operations
  console.log('\n4. Calling encryptAsync...');
  try {
    const encrypted = await instance.encryptAsync('TestPartition', Buffer.from('async test'));
    console.log('✅ encryptAsync succeeded! Result length:', encrypted.length);
    
    console.log('\n5. Calling decryptAsync...');
    const decrypted = await instance.decryptAsync('TestPartition', encrypted);
    console.log('✅ decryptAsync succeeded! Result:', decrypted.toString());
  } catch (e) {
    console.error('❌ Async operation failed:', e.message);
  }

  // 6. Shutdown
  console.log('\n6. Calling shutdownAsync...');
  await instance.shutdownAsync();
  console.log('✅ Shutdown complete');
}

test().catch(console.error);