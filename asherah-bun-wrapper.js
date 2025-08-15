/**
 * Asherah wrapper for Bun compatibility
 * 
 * This wrapper fixes a Go runtime initialization issue in Bun where
 * complex Go modules like Asherah hang if they're the first Go module loaded.
 * The solution is to warm up the Go runtime with a simple module first.
 */

const { dlopen, FFIType } = require('bun:ffi');
const path = require('path');

// Step 1: Warm up Go runtime with a simple module
function warmupGoRuntime() {
    try {
        const warmupPath = path.join(__dirname, 'go-runtime-warmup.dylib');
        const warmupLib = dlopen(warmupPath, {
            WarmupGoRuntime: { returns: FFIType.int, args: [] }
        });
        
        const result = warmupLib.symbols.WarmupGoRuntime();
        if (result !== 1) {
            throw new Error(`Go runtime warmup failed with code: ${result}`);
        }
        
        console.log('[Asherah] Go runtime warmed up successfully');
        return true;
    } catch (e) {
        console.error('[Asherah] Failed to warm up Go runtime:', e.message);
        return false;
    }
}

// Step 2: Load the actual Asherah module
function loadAsherah() {
    // Warm up first
    if (!warmupGoRuntime()) {
        throw new Error('Failed to initialize Go runtime for Asherah');
    }
    
    // Now load Asherah safely
    const asherahPath = path.join(__dirname, 'dist', 'asherah');
    const asherah = require(asherahPath);
    
    console.log('[Asherah] Module loaded successfully');
    return asherah;
}

// Export the wrapped Asherah module
module.exports = loadAsherah();