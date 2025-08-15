#!/usr/bin/env node

// Quick test to verify we can load the module
console.log('Testing module load only...');
console.log('Runtime:', typeof Bun !== 'undefined' ? 'Bun' : 'Node.js');

try {
    const asherah = require('./build/Release/asherah.node');
    console.log('✅ Module loaded successfully');
    console.log('Available methods:', Object.getOwnPropertyNames(asherah).length);
    process.exit(0);
} catch (error) {
    console.error('❌ ERROR loading module:', error.message);
    process.exit(1);
}