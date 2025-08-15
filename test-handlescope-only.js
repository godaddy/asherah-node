#!/usr/bin/env bun

console.log('Testing handle scope fix only (no CGO changes)...');

// Test loading various native modules
console.log('\n1. Testing crypto module:');
const crypto = require('crypto');
const hash = crypto.createHash('sha256').update('test').digest('hex');
console.log('✅ Crypto works:', hash.substring(0, 16) + '...');

console.log('\n2. Testing asherah module load:');
try {
    const asherah = require('./build/Release/asherah.node');
    console.log('✅ Module loaded successfully');
    console.log('Available methods:', Object.keys(asherah).join(', '));
} catch (e) {
    console.error('❌ Failed to load module:', e.message);
}

console.log('\n3. Testing buffer operations:');
const buf = Buffer.from('test');
console.log('✅ Buffer created:', buf.toString());

console.log('\n4. Testing async operations:');
setTimeout(() => {
    console.log('✅ Async timeout works');
}, 100);

setImmediate(() => {
    console.log('✅ setImmediate works');
});

process.nextTick(() => {
    console.log('✅ nextTick works');
});

console.log('\nHandle scope fix test completed!');