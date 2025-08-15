const { dlopen, FFIType, CString, ptr } = require('bun:ffi');

console.log('Testing simple Go library with dlopen...');

try {
  const lib = dlopen('./test-go-init.dylib', {
    TestInit: {
      returns: FFIType.void,
      args: []
    }
  });
  
  console.log('Library loaded, calling TestInit...');
  lib.symbols.TestInit();
  console.log('✅ TestInit succeeded!');
} catch (e) {
  console.error('❌ Error:', e);
}