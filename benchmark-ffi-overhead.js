#!/usr/bin/env bun

console.log('=== FFI Initialization Overhead Benchmark ===\n');

// Measure startup time without FFI
console.log('1. Measuring baseline startup (no FFI)...');
const baselineStart = performance.now();
// Just basic operations
const path = require('path');
const fs = require('fs');
const baselineEnd = performance.now();
console.log(`   Baseline startup: ${(baselineEnd - baselineStart).toFixed(2)}ms`);

// Measure FFI import overhead
console.log('2. Measuring FFI import overhead...');
const ffiImportStart = performance.now();
const { dlopen, FFIType } = require('bun:ffi');
const ffiImportEnd = performance.now();
console.log(`   FFI import: ${(ffiImportEnd - ffiImportStart).toFixed(2)}ms`);

// Measure first dlopen overhead  
console.log('3. Measuring first dlopen overhead...');
const firstDlopenStart = performance.now();
const libPath = path.join(__dirname, 'asherah-bun-preload/lib/noop_warmup.dylib');
const lib = dlopen(libPath, {
    warmup: { returns: FFIType.int, args: [] }
});
const firstDlopenEnd = performance.now();
console.log(`   First dlopen: ${(firstDlopenEnd - firstDlopenStart).toFixed(2)}ms`);

// Measure function call overhead
console.log('4. Measuring FFI function call overhead...');
const callStart = performance.now();
const result = lib.symbols.warmup();
const callEnd = performance.now();
console.log(`   Function call: ${(callEnd - callStart).toFixed(2)}ms`);

// Measure subsequent dlopen overhead
console.log('5. Measuring subsequent dlopen overhead...');
const secondDlopenStart = performance.now();
const lib2 = dlopen(libPath, {
    warmup: { returns: FFIType.int, args: [] }
});
const secondDlopenEnd = performance.now();
console.log(`   Second dlopen: ${(secondDlopenEnd - secondDlopenStart).toFixed(2)}ms`);

// Total FFI initialization cost
const totalFFI = (ffiImportEnd - ffiImportStart) + (firstDlopenEnd - firstDlopenStart);
console.log(`\n📊 Total FFI initialization: ${totalFFI.toFixed(2)}ms`);
console.log(`📊 Overhead vs baseline: ${((totalFFI / (baselineEnd - baselineStart)) * 100).toFixed(1)}%`);

// Compare with Node.js N-API loading
console.log('\n6. Measuring N-API module loading...');
try {
    const napiStart = performance.now();
    const asherah = require('./build/Release/asherah.node');
    const napiEnd = performance.now();
    console.log(`   N-API load: ${(napiEnd - napiStart).toFixed(2)}ms`);
    console.log(`   FFI vs N-API ratio: ${(totalFFI / (napiEnd - napiStart)).toFixed(2)}x`);
} catch (error) {
    console.log(`   N-API load failed: ${error.message.split('\n')[0]}`);
}

console.log(`\n🎯 Result: FFI init adds ${totalFFI.toFixed(2)}ms to startup`);