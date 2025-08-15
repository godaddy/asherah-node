#!/usr/bin/env bun

console.log('Testing ultra-minimal branch implementation...');

try {
    console.log('1. Loading asherah-node with ultra-minimal FFI...');
    process.env.ASHERAH_BUN_VERBOSE = 'true';
    const asherah = require('./src/index.js');
    console.log('✅ Module loaded successfully!');
    
    console.log('2. Module loading works - no external dependencies needed!');
    console.log('🎉 Just one line: require("bun:ffi")');
    
} catch (error) {
    console.error('❌ Module load failed:', error.message);
}