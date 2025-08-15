#!/usr/bin/env node

// Test simpler CGO functions to see if the issue is specific to SetupJson
console.log('Testing simple CGO functions...');
console.log('Runtime:', typeof Bun !== 'undefined' ? 'Bun' : 'Node.js');

const asherah = require('./build/Release/asherah.node');
console.log('Module loaded successfully');

// Test the simplest possible function first
console.log('\n=== Testing get_setup_status (should be safe) ===');
try {
    const status = asherah.get_setup_status();
    console.log('✅ get_setup_status():', status);
} catch (e) {
    console.error('❌ get_setup_status failed:', e.message);
}

// Test setenv function (which calls CGO but doesn't initialize the full runtime)
console.log('\n=== Testing setenv (CGO but no runtime init) ===');
try {
    const start = Date.now();
    asherah.setenv('{"TEST": "value"}');
    const elapsed = Date.now() - start;
    console.log(`✅ setenv completed in ${elapsed}ms`);
} catch (e) {
    console.error('❌ setenv failed:', e.message);
}

// Test EstimateBuffer if it exists
console.log('\n=== Testing other non-setup CGO functions ===');
const methods = Object.getOwnPropertyNames(asherah);
console.log('Available methods:', methods);

// Try to call methods that don't require setup
const safeMethods = methods.filter(m => 
    !m.includes('setup') && 
    !m.includes('encrypt') && 
    !m.includes('decrypt') && 
    !m.includes('shutdown')
);

safeMethods.forEach(method => {
    if (method === 'get_setup_status' || method === 'setenv') return; // Already tested
    
    console.log(`\n--- Testing ${method} ---`);
    try {
        // These should be safe to call without parameters for testing
        if (method.includes('set_max_stack') || method.includes('set_safety')) {
            console.log(`Skipping ${method} (requires parameters)`);
            return;
        }
        
        const start = Date.now();
        const result = asherah[method]();
        const elapsed = Date.now() - start;
        console.log(`✅ ${method}() completed in ${elapsed}ms, result:`, result);
    } catch (e) {
        console.log(`⚠️  ${method}() failed (expected):`, e.message);
    }
});

console.log('\n=== Simple CGO test complete ===');