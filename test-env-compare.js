#!/usr/bin/env node

// Compare environment variables that might affect CGO
console.log('Runtime:', typeof Bun !== 'undefined' ? 'Bun' : 'Node.js');

const cgoVars = [
    'GOMAXPROCS',
    'GOTRACEBACK', 
    'GODEBUG',
    'CGO_ENABLED',
    'DYLD_LIBRARY_PATH',
    'LD_LIBRARY_PATH'
];

console.log('\nCGO-related environment variables:');
for (const varName of cgoVars) {
    const value = process.env[varName];
    console.log(`${varName}: ${value || '(unset)'}`);
}

console.log('\nProcess info:');
console.log('PID:', process.pid);
console.log('Platform:', process.platform);
console.log('Arch:', process.arch);

// Test basic threading
console.log('\nTesting if module loads:');
try {
    const asherah = require('./build/Release/asherah.node');
    console.log('✅ Module loaded, methods:', Object.getOwnPropertyNames(asherah).length);
} catch (error) {
    console.log('❌ Module load failed:', error.message);
}