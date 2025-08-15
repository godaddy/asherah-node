/**
 * asherah-node - Entry point with runtime compatibility support
 * 
 * This module automatically handles runtime compatibility by detecting
 * JavaScript runtimes that need Go initialization and loading the warmup
 * library if available.
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
      if (arch === 'x64') {
        libName = 'go-warmup-darwin-x64.dylib';
      } else if (arch === 'arm64') {
        libName = 'go-warmup-darwin-arm64.dylib';
      } else {
        throw new Error(`Unsupported architecture for Darwin: ${arch}. Only x64 and arm64 are supported.`);
      }
    } else if (platform === 'linux') {
      if (arch === 'x64') {
        libName = 'go-warmup-linux-x64.so';
      } else if (arch === 'arm64') {
        libName = 'go-warmup-linux-arm64.so';
      } else {
        throw new Error(`Unsupported architecture for Linux: ${arch}. Only x64 and arm64 are supported.`);
      }
    } else {
      throw new Error(`Unsupported platform: ${platform}. Only darwin and linux are supported.`);
    }
    
    // Load Go warmup library to initialize Go runtime for compatibility
    const libPath = path.join(__dirname, '..', 'lib', libName);
    
    // Check if library exists
    if (!fs.existsSync(libPath)) {
      throw new Error(
        `asherah-node: Go warmup library not found at ${libPath}. ` +
        `Please ensure you have the latest asherah-cobhan release with warmup library support.`
      );
    }
    
    const lib = dlopen(libPath, {
      Warmup: { returns: FFIType.int, args: [] }
    });
    
    // Initialize Go runtime
    lib.symbols.Warmup();
    
    // Optional: Log success in verbose mode
    if (process.env.ASHERAH_VERBOSE) {
      console.log('✅ asherah-node: Go runtime initialized for compatibility');
    }
    
  } catch (error) {
    // Re-throw the error - initialization is required for Bun
    throw new Error(`asherah-node: Failed to initialize Go runtime for Bun: ${error.message}`);
  }
}

// Load the native asherah module
module.exports = require(path.join(__dirname, '..', 'build', 'Release', 'asherah.node'));
