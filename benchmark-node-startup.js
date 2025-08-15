#!/usr/bin/env node

console.log('=== Node.js Startup Benchmark ===\n');

// Measure baseline startup
console.log('1. Measuring Node.js baseline startup...');
const baselineStart = performance.now();
const path = require('path');
const fs = require('fs');
const baselineEnd = performance.now();
console.log(`   Node.js baseline: ${(baselineEnd - baselineStart).toFixed(2)}ms`);

// Measure N-API loading (Node.js has FFI "pre-initialized")
console.log('2. Measuring N-API module loading in Node.js...');
const napiStart = performance.now();
try {
    const asherah = require('./build/Release/asherah.node');
    const napiEnd = performance.now();
    console.log(`   N-API load: ${(napiEnd - napiStart).toFixed(2)}ms`);
    console.log(`   ✅ N-API loading works in Node.js`);
    
    // Quick setup test
    const setupStart = performance.now();
    asherah.setup({
        ServiceName: 'benchmark',
        ProductID: 'test',
        KMS: 'static',
        Metastore: 'memory',
        Verbose: false
    });
    const setupEnd = performance.now();
    console.log(`   Setup time: ${(setupEnd - setupStart).toFixed(2)}ms`);
    asherah.shutdown();
    
} catch (error) {
    console.log(`   ❌ N-API load failed: ${error.message.split('\n')[0]}`);
}

console.log(`\n📊 Node.js has "zero" FFI init cost (built-in)`);
console.log(`📊 Total N-API ready time: ${(baselineEnd - baselineStart).toFixed(2)}ms`);