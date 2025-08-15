#!/usr/bin/env bun

console.log('Testing Go module hang...');

const asherah = require('./build/Release/asherah.node');
console.log('Module loaded');

// Test synchronous function
console.log('About to call setenv (should be fast)...');
const envData = JSON.stringify({ test: "value" });

// Add timeout to detect hang
const timeout = setTimeout(() => {
    console.error('❌ Function call is hanging!');
    console.log('Stack trace:', new Error().stack);
    process.exit(1);
}, 1000);

try {
    console.log('Calling setenv now...');
    asherah.setenv(envData);
    clearTimeout(timeout);
    console.log('✅ setenv completed!');
} catch (e) {
    clearTimeout(timeout);
    console.error('❌ Error:', e);
}