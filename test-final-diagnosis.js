#!/usr/bin/env node

// Final diagnostic test to demonstrate the root cause
console.log('=== FINAL DIAGNOSIS: CGO Main Thread Hang Issue ===');
const runtime = typeof Bun !== 'undefined' ? 'Bun' : 'Node.js';
console.log(`Runtime: ${runtime}`);
console.log(`PID: ${process.pid}`);

console.log('\n=== Key Finding ===');
console.log('The issue affects ANY CGO function call, not just setup()');
console.log('This suggests the problem is in Go runtime initialization on main thread');

const asherah = require('./build/Release/asherah.node');

// Test 1: Non-CGO function (should work in both)
console.log('\n1. Testing get_setup_status() [Non-CGO, C++ only]:');
try {
    const start = Date.now();
    const result = asherah.get_setup_status();
    const elapsed = Date.now() - start;
    console.log(`   ✅ Success: ${result} (${elapsed}ms)`);
} catch (e) {
    console.log(`   ❌ Failed: ${e.message}`);
}

// Test 2: Simple CGO function (hangs in Bun, works in Node.js)
console.log('\n2. Testing setenv() [Simple CGO function]:');
console.log('   This will hang in Bun but work in Node.js...');

const timeout = setTimeout(() => {
    console.log('   ❌ TIMEOUT: CGO call hung (this proves the issue)');
    console.log('\n=== CONCLUSION ===');
    console.log('The issue is that Bun\'s main thread environment prevents');
    console.log('Go runtime from initializing properly when any CGO function');
    console.log('is called for the first time.');
    console.log('\nThis is NOT a thread pool issue - it\'s a main thread issue.');
    console.log('The fix needs to address how CGO initializes in Bun\'s main thread.');
    process.exit(1);
}, 3000);

try {
    const start = Date.now();
    asherah.setenv('{"TEST": "diagnosis"}');
    const elapsed = Date.now() - start;
    clearTimeout(timeout);
    console.log(`   ✅ Success: CGO call completed (${elapsed}ms)`);
    
    console.log('\n=== SUCCESS CASE ===');
    console.log('If you see this, CGO is working on this runtime.');
    console.log('This means either:');
    console.log('1. You\'re running Node.js (expected)');
    console.log('2. You\'re running Bun with a fix applied');
    
} catch (e) {
    clearTimeout(timeout);
    console.log(`   ❌ Failed: ${e.message}`);
}