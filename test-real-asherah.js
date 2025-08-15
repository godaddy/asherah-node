#!/usr/bin/env bun

console.log('Testing real Asherah module vs test module...\n');

// Test 1: Real Asherah module
console.log('=== Test 1: Real Asherah Module ===');
try {
    console.log('Loading asherah-node from dist...');
    const asherah = require('./dist/asherah');
    console.log('✅ Real Asherah loaded');
    console.log('Real Asherah exports:', Object.keys(asherah));
    
    const config = {
        ServiceName: "test",
        ProductID: "test",
        KMS: "static",
        Metastore: "memory"
    };
    
    console.log('Calling real Asherah setup...');
    try {
        const result = asherah.setup(config);
        console.log('✅ Real Asherah setup succeeded:', result);
    } catch (e) {
        console.log('❌ Real Asherah setup failed:', e.message);
    }
    
} catch (e) {
    console.log('❌ Failed to load real Asherah:', e.message);
}

console.log('\n=== Test 2: Test Go Module ===');
// Test 2: Test Go module (should hang)
try {
    console.log('Loading test Go module...');
    const testGo = require('./go_setup_full.node');
    console.log('✅ Test Go module loaded');
    console.log('Test Go exports:', Object.keys(testGo));
    
    console.log('Calling test Go setupSync...');
    try {
        const result = testGo.setupSync();
        console.log('✅ Test Go setup succeeded:', result);
    } catch (e) {
        console.log('❌ Test Go setup failed:', e.message);
    }
    
} catch (e) {
    console.log('❌ Failed to load test Go:', e.message);
}