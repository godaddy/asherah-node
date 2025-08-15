console.log('Loading module...');
const asherah = require('./build/Release/asherah.node');
console.log('Module loaded');

// Try async first (should work)
console.log('Testing async function...');
asherah.setupAsync('{}')
  .then(() => {
    console.log('✅ Async setup succeeded');
    
    // Now try sync
    console.log('Testing sync function...');
    try {
      asherah.setenv('{}');
      console.log('✅ Sync setenv succeeded');
    } catch (e) {
      console.error('❌ Sync setenv failed:', e.message);
    }
  })
  .catch(e => {
    console.error('❌ Async setup failed:', e.message);
  });