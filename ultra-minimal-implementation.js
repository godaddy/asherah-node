/**
 * asherah-node - Ultra-minimal Bun compatibility
 * 
 * Just importing bun:ffi is sufficient to enable N-API loading!
 * No external libraries, no dlopen calls, no function definitions needed.
 */

const path = require('path');

// Ultra-minimal Bun compatibility - just import bun:ffi
if (typeof Bun !== 'undefined') {
  try {
    // This single import is sufficient to initialize FFI subsystem
    require('bun:ffi');
    
    if (process.env.ASHERAH_BUN_VERBOSE) {
      console.log('✅ asherah-node: Bun FFI subsystem initialized (import-only)');
    }
  } catch (error) {
    if (process.env.ASHERAH_BUN_VERBOSE) {
      console.warn('⚠️  asherah-node: Bun FFI import failed:', error.message);
    }
  }
}

// Load the native asherah module
module.exports = require(path.join(__dirname, 'build', 'Release', 'asherah.node'));