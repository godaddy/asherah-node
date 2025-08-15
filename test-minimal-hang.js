#!/usr/bin/env bun

const { dlopen, FFIType } = require('bun:ffi');

console.log('Testing minimal hang scenarios...\n');

const lib = dlopen('./test-minimal-hang.dylib', {
    SimpleWork: { returns: FFIType.int, args: [] },
    GoroutineWork: { returns: FFIType.int, args: [] },
    ChannelWork: { returns: FFIType.int, args: [] },
    RuntimeWork: { returns: FFIType.int, args: [] },
});

const tests = [
    { name: 'SimpleWork', desc: 'Basic work with sleep' },
    { name: 'GoroutineWork', desc: 'Work using goroutines and WaitGroup' },
    { name: 'ChannelWork', desc: 'Work using channels' },
    { name: 'RuntimeWork', desc: 'Work using runtime operations (GC, etc)' },
];

async function runTest(test) {
    console.log(`\n=== Testing ${test.name}: ${test.desc} ===`);
    console.log('Starting...');
    
    try {
        const start = Date.now();
        const result = lib.symbols[test.name]();
        const elapsed = Date.now() - start;
        console.log(`✅ ${test.name} succeeded: ${result} (${elapsed}ms)`);
        return true;
    } catch (e) {
        console.error(`❌ ${test.name} failed: ${e.message}`);
        return false;
    }
}

// Run tests one by one
async function runAllTests() {
    for (const test of tests) {
        const success = await runTest(test);
        if (!success) {
            console.log(`\n⚠️  ${test.name} failed - stopping tests`);
            break;
        }
        
        // Small delay between tests
        await new Promise(resolve => setTimeout(resolve, 100));
    }
    
    console.log('\n=== Test Complete ===');
    process.exit(0);
}

runAllTests().catch(console.error);