#!/usr/bin/env node

console.log('Testing signal mask fix for CGO compatibility...');
console.log('Runtime:', process.versions.bun ? 'Bun' : 'Node.js');

// Test basic module loading first  
try {
    console.log('Loading asherah module...');
    const asherah = require('./build/Release/asherah.node');
    console.log('✅ Module loaded successfully');
    
    // Test synchronous CGO call
    console.log('Testing synchronous CGO call...');
    const testConfig = JSON.stringify({
        "test": "signal_mask_fix",
        "runtime": process.versions.bun ? "bun" : "node"
    });
    
    console.log('Calling setenv with test config...');
    asherah.setenv(testConfig);
    console.log('✅ Synchronous CGO call completed successfully!');
    
    console.log('Signal mask fix verification: PASS');
    
} catch (error) {
    console.error('❌ Signal mask fix verification failed:', error.message);
    process.exit(1);
}