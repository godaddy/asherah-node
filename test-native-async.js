#!/usr/bin/env bun

console.log('Testing native module async operations...');

// Test with a different native module that uses async
try {
    // First test the built-in crypto which uses native async
    const crypto = require('crypto');
    
    console.log('Testing crypto.randomBytes async...');
    crypto.randomBytes(32, (err, buffer) => {
        if (err) {
            console.error('❌ Crypto async failed:', err);
        } else {
            console.log('✅ Crypto async works:', buffer.toString('hex').substring(0, 16) + '...');
        }
    });
    
    // Also test with promise
    const util = require('util');
    const randomBytesAsync = util.promisify(crypto.randomBytes);
    
    randomBytesAsync(32).then(buffer => {
        console.log('✅ Crypto promise works:', buffer.toString('hex').substring(0, 16) + '...');
    }).catch(err => {
        console.error('❌ Crypto promise failed:', err);
    });
    
    // Keep process alive
    setTimeout(() => {
        console.log('Test completed');
    }, 100);
    
} catch (e) {
    console.error('Error:', e);
}