#!/usr/bin/env bun

console.log('Testing N-API CGO bridge...\n');

try {
    const bridge = require('./test-napi-cgo-bridge.node');
    console.log('Bridge loaded:', Object.keys(bridge));
    
    console.log('\nCalling testSetupJson...');
    const result = bridge.testSetupJson();
    console.log('Result:', result);
    
} catch (e) {
    console.error('Error:', e.message);
}
