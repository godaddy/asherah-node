/**
 * bun-go-runtime-init
 * 
 * Initializes the Go runtime in Bun to prevent hangs with CGO modules.
 * This must be loaded before any Go-based native modules.
 */

const { dlopen, FFIType } = require('bun:ffi');
const path = require('path');
const fs = require('fs');

// Global initialization flag
const INIT_KEY = Symbol.for('bun.go.runtime.initialized');

// Only initialize once per process
if (!global[INIT_KEY]) {
    const platform = process.platform;
    const arch = process.arch;
    
    // Determine the correct binary for this platform
    let binaryName;
    if (platform === 'darwin') {
        binaryName = 'go-runtime-warmup.dylib';
    } else if (platform === 'linux') {
        binaryName = 'go-runtime-warmup.so';
    } else if (platform === 'win32') {
        binaryName = 'go-runtime-warmup.dll';
    } else {
        console.warn(`[bun-go-runtime-init] Unsupported platform: ${platform}`);
        process.exit(0);
    }
    
    const binaryPath = path.join(__dirname, 'lib', `${platform}-${arch}`, binaryName);
    
    try {
        // Check if binary exists
        if (!fs.existsSync(binaryPath)) {
            // Try fallback path
            const fallbackPath = path.join(__dirname, 'lib', binaryName);
            if (fs.existsSync(fallbackPath)) {
                binaryPath = fallbackPath;
            } else {
                throw new Error(`Binary not found: ${binaryPath}`);
            }
        }
        
        // Load the warmup module
        const lib = dlopen(binaryPath, {
            WarmupGoRuntime: { 
                returns: FFIType.int, 
                args: [] 
            }
        });
        
        // Call the warmup function
        const result = lib.symbols.WarmupGoRuntime();
        
        if (result !== 1) {
            throw new Error(`Warmup returned unexpected value: ${result}`);
        }
        
        // Mark as initialized
        global[INIT_KEY] = true;
        
        // Set environment flag
        process.env.BUN_GO_RUNTIME_INITIALIZED = '1';
        
        if (process.env.BUN_GO_RUNTIME_DEBUG) {
            console.log('[bun-go-runtime-init] ✅ Go runtime initialized successfully');
        }
        
    } catch (error) {
        // Don't fail hard - let the app continue
        // Some Go modules might still work
        if (process.env.BUN_GO_RUNTIME_DEBUG) {
            console.error('[bun-go-runtime-init] ⚠️  Warning:', error.message);
            console.error('[bun-go-runtime-init] Some Go modules may not work correctly');
        }
    }
}

// Public API
module.exports = {
    /**
     * Check if Go runtime has been initialized
     */
    isInitialized() {
        return global[INIT_KEY] === true;
    },
    
    /**
     * Get initialization status details
     */
    getStatus() {
        return {
            initialized: global[INIT_KEY] === true,
            platform: process.platform,
            arch: process.arch,
            version: '1.0.0'
        };
    }
};