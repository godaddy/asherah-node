#!/usr/bin/env bun

console.log('Testing CGO with thread info...');
console.log('Main thread ID:', process.pid);

// Force some native module operations before loading
const crypto = require('crypto');
console.log('Loaded crypto module');

// Import the asherah module
console.log('About to load asherah module...');
const asherah = require('./build/Release/asherah.node');
console.log('Module loaded successfully');

// Just try to call setup with minimal work
try {
    console.log('Setting up minimal config...');
    const config = JSON.stringify({
        "ServiceName": "test", 
        "ProductID": "test",
        "Verbose": false
    });
    
    console.log('About to call setup...');
    setTimeout(() => {
        console.log('Setup appears to be hanging - killing process');
        process.exit(1);
    }, 2000);
    
    asherah.setup(config);
    console.log('✅ Setup completed!');
    process.exit(0);
} catch (e) {
    console.error('❌ Error:', e.message);
    process.exit(1);
}