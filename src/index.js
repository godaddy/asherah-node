/**
 * asherah-node - Unified entry point with Bun runtime support
 * 
 * This module automatically handles Bun compatibility by detecting the runtime
 * and initializing Go warmup if needed, then loading the native module.
 */

const path = require('path');

// Auto-detect and handle Bun runtime compatibility
if (typeof Bun !== 'undefined') {
  try {
    const { dlopen, FFIType } = require('bun:ffi');
    
    // Load minimal C library to initialize FFI subsystem for Bun compatibility  
    const libPath = path.join(__dirname, '..', 'asherah-bun-preload', 'lib', 'noop_warmup.dylib');
    const lib = dlopen(libPath, {
      warmup: { returns: FFIType.int, args: [] }
    });
    
    // Initialize FFI subsystem
    lib.symbols.warmup();
    
    // Optional: Log success in verbose mode
    if (process.env.ASHERAH_BUN_VERBOSE) {
      console.log('✅ asherah-node: Bun runtime compatibility initialized');
    }
    
  } catch (error) {
    // Warn but continue - user may still be able to use it
    if (process.env.ASHERAH_BUN_VERBOSE) {
      console.warn('⚠️  asherah-node: Bun compatibility initialization failed:', error.message);
    }
  }
}

// Load the native asherah module
module.exports = require(path.join(__dirname, '..', 'build', 'Release', 'asherah.node'));