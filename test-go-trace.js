#!/usr/bin/env bun

const addon = require('./go_trace.node');

console.log('Testing Go runtime initialization...');

try {
    const result = addon.testRuntime();
    console.log('Success! Result:', result);
} catch (e) {
    console.error('Error:', e.message);
}

// Also test async
console.log('\nTesting async...');
addon.testAsync().then(r => {
    console.log('Async success:', r);
}).catch(e => {
    console.error('Async error:', e);
});

setTimeout(() => {
    console.log('Timeout - exiting');
    process.exit(1);
}, 5000);