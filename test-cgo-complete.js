#!/usr/bin/env bun

console.log('Testing complete CGO solution...');
console.log('Runtime:', process.versions.bun ? 'Bun' : 'Node.js');

// Import the asherah module
const asherah = require('./build/Release/asherah.node');

console.log('✅ Module loaded successfully');

// Test basic Go function call
console.log('Testing Go SetEnv function...');

const testEnv = JSON.stringify({
    "test": "value",
    "another": "test_value"
});

try {
    console.log('About to call Go SetEnv function...');
    asherah.setenv(testEnv);
    console.log('✅ Go SetEnv function completed successfully!');
    console.log('✅ CGO initialization is working correctly');
} catch (e) {
    console.error('❌ Go SetEnv function failed:', e.message);
    process.exit(1);
}

console.log('Complete CGO solution test passed!');