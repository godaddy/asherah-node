#!/usr/bin/env node

const asherah = require('./build/Release/asherah.node');

console.log('Testing async-only Go function calls...');
console.log('Runtime:', process.versions.bun ? 'Bun' : 'Node.js');

// Only test async setup to see if it hangs
const config = {
    "ProductID": "test-product", 
    "ServiceName": "test-service",
    "Verbose": true,
    "Metastore": "memory"
};

console.log('Calling setup_async...');
const start = Date.now();

const promise = asherah.setup_async(config);

// Add timeout to detect hang
const timeout = setTimeout(() => {
    console.error('❌ setup_async timed out after 10 seconds - likely hanging');
    process.exit(1);
}, 10000);

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