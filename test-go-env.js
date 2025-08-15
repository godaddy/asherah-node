// Test with Go runtime environment variables
console.log('Testing with Go runtime environment variables...');

// Set Go runtime environment variables that might help
process.env.GODEBUG = 'asyncpreemptoff=1';  // Disable async preemption
process.env.GOMAXPROCS = '1';  // Limit to single processor
process.env.GOGC = 'off';  // Disable garbage collection initially

console.log('Environment set:');
console.log('  GODEBUG =', process.env.GODEBUG);
console.log('  GOMAXPROCS =', process.env.GOMAXPROCS);
console.log('  GOGC =', process.env.GOGC);

const asherah = require('./build/Release/asherah.node');
console.log('\nModule loaded');

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

console.log('\nCalling setup_async...');
asherah.setup_async(config)
  .then(result => {
    console.log('✅ Setup succeeded:', result);
  })
  .catch(err => {
    console.error('❌ Setup failed:', err.message);
  });