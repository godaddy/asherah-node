console.log('Testing setup function...');

const asherah = require('./build/Release/asherah.node');
console.log('Module loaded, type:', typeof asherah);
console.log('Module contents:', asherah);

// Check if setup exists
console.log('setup function:', asherah.setup);
console.log('typeof setup:', typeof asherah.setup);

if (asherah.setup) {
  console.log('Calling setup...');
  try {
    const result = asherah.setup('{}');
    console.log('Setup returned:', result);
    console.log('Result type:', typeof result);
    
    if (result && typeof result.then === 'function') {
      console.log('It is a promise!');
      result.then(val => {
        console.log('Promise resolved:', val);
      }).catch(err => {
        console.log('Promise rejected:', err);
      });
    }
  } catch (e) {
    console.error('Setup threw:', e.message);
  }
} else {
  console.log('No setup function found');
  console.log('Available properties:', Object.keys(asherah));
  console.log('Available methods:', Object.getOwnPropertyNames(asherah));
}