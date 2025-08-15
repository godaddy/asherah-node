#!/usr/bin/env bun

const addon = require('./go_subprocess.node');

console.log('Testing Go with subprocess...');

// Test subprocess approach
console.log('1. Testing subprocess...');
try {
    const result = addon.subprocess();
    console.log('✅ Subprocess result:', result);
} catch (e) {
    console.error('❌ Subprocess error:', e.message);
}

// Test direct approach (will hang)
console.log('\n2. Testing direct call (will hang)...');
setTimeout(() => {
    console.log('Timeout - direct call hung as expected');
    process.exit(0);
}, 3000);

try {
    const result = addon.preinit();
    console.log('✅ Direct result:', result);
} catch (e) {
    console.error('❌ Direct error:', e.message);
}
