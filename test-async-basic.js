#!/usr/bin/env node

console.log('Testing basic async functionality...');
console.log('Runtime:', process.versions.bun ? 'Bun' : 'Node.js');

// Test async operations that don't involve Go
async function testAsync() {
    try {
        // Test 1: Promise resolution
        const result1 = await Promise.resolve('Promise works');
        console.log('✅', result1);
        
        // Test 2: setTimeout
        const result2 = await new Promise(resolve => {
            setTimeout(() => resolve('setTimeout works'), 100);
        });
        console.log('✅', result2);
        
        // Test 3: Async crypto
        const crypto = require('crypto');
        const result3 = await new Promise((resolve, reject) => {
            crypto.randomBytes(16, (err, buf) => {
                if (err) reject(err);
                else resolve('Async crypto works');
            });
        });
        console.log('✅', result3);
        
        console.log('All async tests completed successfully');
        
    } catch (e) {
        console.error('❌ Async test failed:', e.message);
    }
}

testAsync();