#!/usr/bin/env bun

console.log('Testing just the minimal warmup library...');

const { dlopen, FFIType } = require('bun:ffi');

try {
    const lib = dlopen('./src/bun_warmup_minimal.dylib', {
        MinimalWarmup: { returns: FFIType.int, args: [] }
    });
    
    console.log('Library loaded, calling MinimalWarmup...');
    const result = lib.symbols.MinimalWarmup();
    console.log('✅ MinimalWarmup returned:', result);
} catch (error) {
    console.error('Error:', error);
}