#!/usr/bin/env bun

// Warm up Go runtime by loading a minimal Go library
const { dlopen, FFIType } = require('bun:ffi');

try {
    const lib = dlopen(__dirname + '/bun_warmup_minimal.dylib', {
        MinimalWarmup: { returns: FFIType.int, args: [] }
    });
    
    const result = lib.symbols.MinimalWarmup();
    console.log('✅ Go runtime warmed up successfully:', result);
} catch (error) {
    console.error('⚠️ Warmup failed:', error);
}

module.exports = { warmed: true };