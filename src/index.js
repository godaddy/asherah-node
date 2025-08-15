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
    const fs = require('fs');
    
    // Determine platform-specific library name
    const platform = process.platform;
    const arch = process.arch;
    let libName;
    
    if (platform === 'darwin') {
      libName = arch === 'x64' ? 'bun_warmup_minimal-darwin-x64.dylib' : 'bun_warmup_minimal-darwin-arm64.dylib';
    } else if (platform === 'linux') {
      libName = arch === 'x64' ? 'bun_warmup_minimal-linux-x64.so' : 'bun_warmup_minimal-linux-arm64.so';
    } else {
      throw new Error(`Unsupported platform: ${platform}`);
    }
    
    // Load minimal Go library to initialize Go runtime for Bun compatibility
    const libPath = path.join(__dirname, '..', 'lib', libName);
    
    // Check if library exists (might not be available in older asherah-cobhan releases)
    if (!fs.existsSync(libPath)) {
      console.warn('⚠️  asherah-node: Bun warmup library not found. This may be an older release without Bun support.');
      console.warn('⚠️  asherah-node: Package may not work correctly in Bun runtime');
    } else {
      const lib = dlopen(libPath, {
        Warmup: { returns: FFIType.int, args: [] }
      });
      
      // Initialize Go runtime
      lib.symbols.Warmup();
      
      // Optional: Log success in verbose mode
      if (process.env.ASHERAH_BUN_VERBOSE) {
        console.log('✅ asherah-node: Bun runtime compatibility initialized');
      }
    }
    
  } catch (error) {
    // For errors, warn but continue 
    console.warn('⚠️  asherah-node: Bun compatibility initialization failed:', error.message);
    console.warn('⚠️  asherah-node: Package may not work correctly in Bun runtime');
  }
}

// Load the native asherah module
module.exports = require(path.join(__dirname, '..', 'build', 'Release', 'asherah.node'));