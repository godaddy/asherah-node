#!/usr/bin/env node

const asherah = require('./build/Release/asherah.node');

console.log('Testing Go function calls in different contexts...');
console.log('Runtime:', process.versions.bun ? 'Bun' : 'Node.js');

// Test 1: Simple synchronous SetEnv call 
console.log('\n=== Test 1: Synchronous SetEnv ===');
try {
    const config = JSON.stringify({
        "ProductID": "test",
        "ServiceName": "test", 
        "Verbose": true
    });
    console.log('Calling setenv synchronously...');
    const start = Date.now();
    asherah.setenv(config);
    const end = Date.now();
    console.log('✅ setenv completed in', end - start, 'ms');
} catch (e) {
    console.error('❌ setenv failed:', e.message);
}

// Test 2: Try setup synchronously 
console.log('\n=== Test 2: Synchronous Setup ===');
try {
    const config = {
        "ProductID": "test-product",
        "ServiceName": "test-service",
        "Verbose": true,
        "Metastore": "memory"
    };
    console.log('Calling setup synchronously...');
    const start = Date.now();
    asherah.setup(config);
    const end = Date.now();
    console.log('✅ setup completed in', end - start, 'ms');
} catch (e) {
    console.error('❌ setup failed:', e.message);
}

// Test 3: Try setup_async to confirm hang location
console.log('\n=== Test 3: Async Setup (Expected to hang in Bun) ===');
try {
    const config = {
        "ProductID": "test-product2", 
        "ServiceName": "test-service2",
        "Verbose": true,
        "Metastore": "memory"
    };
    console.log('Calling setup_async...');
    const start = Date.now();
    
    const promise = asherah.setup_async(config);
    
    // Add timeout to detect hang
    const timeout = setTimeout(() => {
        console.error('❌ setup_async timed out after 5 seconds - likely hanging');
        process.exit(1);
    }, 5000);
    
    promise.then(() => {
        clearTimeout(timeout);
        const end = Date.now();
        console.log('✅ setup_async completed in', end - start, 'ms');
        process.exit(0);
    }).catch(e => {
        clearTimeout(timeout);
        console.error('❌ setup_async failed:', e.message);
        process.exit(1);
    });
    
} catch (e) {
    console.error('❌ setup_async failed immediately:', e.message);
    process.exit(1);
}