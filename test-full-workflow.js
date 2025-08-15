const { Asherah } = require('./index.js');

async function testWorkflow() {
  const asherah = new Asherah();
  
  console.log('1. Testing async setup...');
  try {
    await asherah.setup({
      KMS: 'static',
      Metastore: 'memory',
      ServiceName: 'TestService',
      ProductID: 'TestProduct',
      Verbose: true,
      ExpireAfter: 60,
      CheckInterval: 30,
      StaticKeyManagement: {
        KEY_ENCRYPTION_KEY: 'thisIsAStaticMasterKeyForTesting',
      },
    });
    console.log('✅ Async setup succeeded');
  } catch (e) {
    console.error('❌ Async setup failed:', e.message);
    return;
  }

  console.log('\n2. Testing sync encrypt after async setup...');
  try {
    const encrypted = asherah.encrypt('TestPartition', Buffer.from('test data'));
    console.log('✅ Sync encrypt succeeded! Result:', encrypted);
    
    console.log('\n3. Testing sync decrypt...');
    const decrypted = asherah.decrypt('TestPartition', encrypted);
    console.log('✅ Sync decrypt succeeded! Result:', decrypted.toString());
    
  } catch (e) {
    console.error('❌ Sync operation failed:', e.message);
  }

  console.log('\n4. Testing async encrypt...');
  try {
    const encrypted = await asherah.encryptAsync('TestPartition', Buffer.from('async test data'));
    console.log('✅ Async encrypt succeeded! Result:', encrypted);
    
    console.log('\n5. Testing async decrypt...');
    const decrypted = await asherah.decryptAsync('TestPartition', encrypted);
    console.log('✅ Async decrypt succeeded! Result:', decrypted.toString());
  } catch (e) {
    console.error('❌ Async operation failed:', e.message);
  }

  console.log('\n6. Testing shutdown...');
  await asherah.shutdown();
  console.log('✅ Shutdown complete');
}

testWorkflow().catch(console.error);