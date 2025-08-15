#!/usr/bin/env bun

const { dlopen, FFIType, ptr } = require('bun:ffi');

console.log('Testing if preloading Go modules fixes Asherah...\n');

// Step 1: Load a simple Go module first to initialize Go runtime
console.log('Step 1: Preloading simple Go module...');
const simpleLib = dlopen('./test-minimal-hang.dylib', {
    SimpleWork: { returns: FFIType.int, args: [] }
});

// Call it to ensure Go runtime is initialized
const warmup = simpleLib.symbols.SimpleWork();
console.log('✅ Go runtime warmed up:', warmup);

// Step 2: Now try loading the problematic Asherah module
console.log('\nStep 2: Loading Asherah module...');
const asherah = require('./go_setup_full.node');
console.log('✅ Asherah module loaded:', Object.keys(asherah));

// Step 3: Call SetupJson
console.log('\nStep 3: Calling setupSync...');
try {
    const result = asherah.setupSync();
    console.log('✅ setupSync succeeded:', result);
} catch (e) {
    console.error('❌ setupSync failed:', e.message);
}

console.log('\n=== Test complete ===');