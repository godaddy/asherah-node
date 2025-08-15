/**
 * asherah-node - Unified entry point with Bun runtime support
 * 
 * Automatically detects Bun runtime and initializes FFI subsystem if needed.
 */

// Initialize Bun FFI subsystem for N-API compatibility
if (typeof Bun !== 'undefined') {
  try {
    require('bun:ffi');
    
    if (process.env.ASHERAH_BUN_VERBOSE) {
      console.log('✅ asherah-node: Bun FFI initialized');
    }
  } catch (error) {
    // Always log FFI initialization failures - this will break N-API loading
    console.error('❌ asherah-node: Bun FFI initialization failed:', error.message);
    console.error('   N-API modules may fail to load. Ensure Bun version supports FFI.');
  }
}

module.exports = require('../build/Release/asherah.node');