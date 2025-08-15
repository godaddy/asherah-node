#!/usr/bin/env bun

console.log('=== Testing Both Warmup Approaches in Bun ===\n');

console.log('1. Testing Static Initializer Approach:');
try {
    const staticInit = require('./test_static_init.node');
    console.log('✅ Static initializer module loaded successfully');
    console.log('Result:', staticInit.test());
} catch (error) {
    console.error('❌ Static initializer failed:', error.message);
}

console.log('\n2. Testing Library Entry Point Approach:');
try {
    const libraryEntry = require('./test_library_entry.node');
    console.log('✅ Library entry point module loaded successfully');
    console.log('Result:', libraryEntry.test());
} catch (error) {
    console.error('❌ Library entry point failed:', error.message);
}

console.log('\n=== Testing Complete ===');