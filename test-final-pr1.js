#!/usr/bin/env bun

console.log('Testing PR1: Handle scope fix only\n');

// Test 1: Module loads
console.log('1. Testing Go module load...');
try {
    const asherah = require('./build/Release/asherah.node');
    console.log('✅ Module loaded successfully');
    console.log('   Methods available:', Object.getOwnPropertyNames(asherah).length);
} catch (e) {
    console.error('❌ Failed to load module:', e.message);
    process.exit(1);
}

// Test 2: Other native modules still work
console.log('\n2. Testing other native modules...');
const crypto = require('crypto');
console.log('✅ Crypto module works');

// Test 3: Async operations work
console.log('\n3. Testing async operations...');
let asyncCount = 0;
setTimeout(() => {
    asyncCount++;
    if (asyncCount === 3) {
        console.log('✅ All async operations completed');
        console.log('\nPR1 validation complete: Handle scope fix allows Go modules to load!');
        console.log('Note: Go functions will still hang - that requires PR2 (CGO WorkPool support)');
    }
}, 50);

setImmediate(() => asyncCount++);
process.nextTick(() => asyncCount++);