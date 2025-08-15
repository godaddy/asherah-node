#!/usr/bin/env bun

console.log('=== Testing Full SetupJson in Bun ===\n');

const addon = require('./go_setup_full.node');

// Test sync
console.log('1. Testing synchronous SetupJson...');
try {
    const result = addon.setupSync();
    console.log(`✅ Sync SetupJson succeeded! Result: ${result}\n`);
} catch (e) {
    console.error(`❌ Sync SetupJson failed: ${e.message}\n`);
}

// Test async  
console.log('2. Testing asynchronous SetupJson...');
addon.setupAsync()
    .then(result => {
        console.log(`✅ Async SetupJson succeeded! Result: ${result}`);
        process.exit(0);
    })
    .catch(e => {
        console.error(`❌ Async SetupJson failed: ${e.message}`);
        process.exit(1);
    });

setTimeout(() => {
    console.error('⏰ Async SetupJson timed out after 5 seconds');
    process.exit(1);
}, 5000);