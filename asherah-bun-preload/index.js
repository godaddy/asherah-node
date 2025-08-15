/**
 * asherah-bun-preload - Bun runtime compatibility module
 * 
 * This module provides access to the minimal Go warmup library
 * for direct use if needed, though most users should use the main
 * asherah-node package which includes this automatically.
 */

const path = require('path');

// Export the path to the preload library for advanced use cases
const getPreloadLibraryPath = () => {
  const platform = process.platform;
  let libExt;
  
  switch (platform) {
    case 'darwin':
      libExt = '.dylib';
      break;
    case 'linux':
      libExt = '.so';
      break;
    case 'win32':
      libExt = '.dll';
      break;
    default:
      throw new Error(`Unsupported platform: ${platform}`);
  }
  
  return path.join(__dirname, 'lib', `bun_warmup_minimal${libExt}`);
};

module.exports = {
  getPreloadLibraryPath
};