/**
 * Bun Go Runtime Initializer
 * 
 * This module fixes Go runtime initialization issues in Bun.
 * Load this ONCE at application startup before using any Go-based native modules.
 * 
 * Usage:
 *   // At the top of your main file:
 *   require('bun-go-runtime-init');
 *   
 *   // Then use Go modules normally:
 *   const asherah = require('asherah');
 */

const { dlopen, FFIType } = require('bun:ffi');
const path = require('path');
const fs = require('fs');

// Global flag to ensure we only initialize once
const INIT_KEY = Symbol.for('bun.go.runtime.initialized');

if (!global[INIT_KEY]) {
    // Embedded Go runtime initializer (base64 encoded minimal Go shared library)
    // This is a tiny (~50KB) module that just initializes the Go runtime
    const WARMUP_MODULE_BASE64 = `
    // This would contain the base64-encoded go-runtime-warmup.dylib
    // For now, we'll load from file
    `;

    function initializeGoRuntime() {
        try {
            // Option 1: Try to load embedded module from base64 (production)
            // const buffer = Buffer.from(WARMUP_MODULE_BASE64, 'base64');
            // ... write to temp file and load ...
            
            // Option 2: Load from installed package (current implementation)
            const warmupPath = path.join(__dirname, 'go-runtime-warmup.dylib');
            
            if (!fs.existsSync(warmupPath)) {
                console.warn('[bun-go-runtime-init] Warmup module not found, Go modules may hang');
                return false;
            }
            
            const warmupLib = dlopen(warmupPath, {
                WarmupGoRuntime: { returns: FFIType.int, args: [] }
            });
            
            const result = warmupLib.symbols.WarmupGoRuntime();
            if (result !== 1) {
                throw new Error(`Go runtime warmup failed with code: ${result}`);
            }
            
            // Mark as initialized
            global[INIT_KEY] = true;
            
            // Optional: Set process flag for debugging
            process.env.BUN_GO_RUNTIME_INITIALIZED = '1';
            
            if (process.env.DEBUG) {
                console.log('[bun-go-runtime-init] Go runtime initialized successfully');
            }
            
            return true;
        } catch (e) {
            console.error('[bun-go-runtime-init] Failed to initialize Go runtime:', e.message);
            return false;
        }
    }
    
    // Initialize immediately when module is loaded
    initializeGoRuntime();
} else {
    if (process.env.DEBUG) {
        console.log('[bun-go-runtime-init] Go runtime already initialized');
    }
}

// Export a function to check initialization status
module.exports = {
    isInitialized: () => global[INIT_KEY] === true,
    version: '1.0.0'
};