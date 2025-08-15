#!/usr/bin/env bun

const addon = require('./go_init_direct.node');

console.log('Testing direct Go runtime initialization...\n');

// Test 1: Direct init
console.log('1. Testing direct init...');
try {
    const result = addon.directInit();
    console.log('✅ Direct init succeeded! Result:', result);
} catch (e) {
    console.error('❌ Direct init failed:', e.message);
}

// Test 2: Thread init
console.log('\n2. Testing thread-based init...');
try {
    const result = addon.threadInit();
    console.log('✅ Thread init succeeded! Result:', result);
} catch (e) {
    console.error('❌ Thread init failed:', e.message);
}

setTimeout(() => {
    console.log('\nTimeout - exiting');
    process.exit(1);
}, 5000);