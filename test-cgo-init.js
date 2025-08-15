#!/usr/bin/env bun

const { dlopen, FFIType } = require('bun:ffi');

console.log('Testing CGO initialization sequence...\n');

const lib = dlopen('./test-cgo-init.dylib', {
    TestInit: { returns: FFIType.int, args: [] },
    TestInitWithGoroutine: { returns: FFIType.int, args: [] },
    TestRuntimeInit: { returns: FFIType.int, args: [] },
});

async function runTest(name) {
    console.log(`\n=== Testing ${name} ===`);
    try {
        const start = Date.now();
        const result = lib.symbols[name]();
        const elapsed = Date.now() - start;
        console.log(`✅ ${name} returned: ${result} (${elapsed}ms)`);
        return true;
    } catch (e) {
        console.error(`❌ ${name} failed: ${e.message}`);
        return false;
    }
}

async function main() {
    await runTest('TestInit');
    await new Promise(r => setTimeout(r, 100));
    
    await runTest('TestInitWithGoroutine');
    await new Promise(r => setTimeout(r, 100));
    
    await runTest('TestRuntimeInit');
    
    console.log('\n=== All tests complete ===');
}

main().catch(console.error);
