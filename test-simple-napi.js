#!/usr/bin/env node

// Test the most basic N-API functionality to isolate the issue
console.log('Testing basic N-API functionality...');
console.log('Runtime:', typeof Bun !== 'undefined' ? 'Bun' : 'Node.js');

const asherah = require('./build/Release/asherah.node');
console.log('Module loaded successfully');

// Test that doesn't involve CGO at all - just the module's basic exports
console.log('Available methods:');
const methods = Object.getOwnPropertyNames(asherah);
methods.forEach(method => {
    console.log(`  - ${method}: ${typeof asherah[method]}`);
});

// Try to call a simple method that might not involve CGO
console.log('\nTesting method calls...');
console.log('Note: The hang might be in the N-API layer itself, not CGO specifically');

process.exit(0);