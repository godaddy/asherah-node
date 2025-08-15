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
    // FFI initialization failure is fatal under Bun - N-API modules cannot load
    throw new Error(
      `asherah-node: Bun FFI initialization failed: ${error.message}\n` +
      `N-API modules cannot load without FFI support. Please ensure your Bun version supports the 'bun:ffi' module.`
    );
  }
}

module.exports = require('../build/Release/asherah.node');