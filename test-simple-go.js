const ffi = require('ffi-napi');
const path = require('path');

console.log('Loading simple Go library...');

try {
  const lib = ffi.Library(path.join(__dirname, 'test-go-init.dylib'), {
    'TestInit': ['void', []]
  });
  
  console.log('Library loaded, calling TestInit...');
  lib.TestInit();
  console.log('✅ TestInit succeeded!');
} catch (e) {
  console.error('❌ Error:', e.message);
}