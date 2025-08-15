#!/usr/bin/env node

const { execSync } = require('child_process');
const fs = require('fs');

console.log('=== Runtime Comparison Analysis ===');
const runtime = typeof Bun !== 'undefined' ? 'Bun' : 'Node.js';
console.log('Runtime:', runtime);

// Run the native thread analysis
console.log('\n=== Native Thread Analysis ===');
try {
    const output = execSync('./test-thread-analysis', { encoding: 'utf8' });
    console.log(output);
} catch (e) {
    console.log('Error running native analysis:', e.message);
}

// Check JavaScript environment specifics
console.log('\n=== JavaScript Environment ===');
console.log('Global object keys:', Object.getOwnPropertyNames(globalThis).length);

// Check for runtime-specific globals
const nodeGlobals = ['process', 'Buffer', 'global', '__dirname', '__filename'];
const bunGlobals = ['Bun', 'ReadableStream', 'WritableStream'];

console.log('\nNode.js globals present:');
nodeGlobals.forEach(g => {
    console.log(`  ${g}: ${typeof globalThis[g] !== 'undefined'}`);
});

console.log('\nBun globals present:');
bunGlobals.forEach(g => {
    console.log(`  ${g}: ${typeof globalThis[g] !== 'undefined'}`);
});

// Check process-level capabilities
console.log('\n=== Process Capabilities ===');
if (typeof process !== 'undefined') {
    console.log('Process features:');
    console.log(`  Platform: ${process.platform}`);
    console.log(`  Arch: ${process.arch}`);
    console.log(`  Node version: ${process.version || 'N/A'}`);
    
    if (process.features) {
        console.log('  Features:', Object.keys(process.features));
    }
    
    // Check if we can install signal handlers
    console.log('\n=== Signal Handler Test ===');
    try {
        let sigprofHandled = false;
        const handler = () => { sigprofHandled = true; };
        
        process.on('SIGPROF', handler);
        console.log('✅ SIGPROF handler installed successfully');
        
        // Clean up
        process.removeListener('SIGPROF', handler);
    } catch (e) {
        console.log('❌ SIGPROF handler failed:', e.message);
    }
}

// Test native module loading
console.log('\n=== Native Module Loading ===');
try {
    const start = Date.now();
    const asherah = require('./build/Release/asherah.node');
    const loadTime = Date.now() - start;
    console.log(`✅ Native module loaded in ${loadTime}ms`);
    console.log(`Methods available: ${Object.getOwnPropertyNames(asherah).length}`);
    
    // Test a simple synchronous call that doesn't involve Go runtime
    try {
        const status = asherah.get_setup_status();
        console.log(`✅ Simple native call works: setup_status=${status}`);
    } catch (e) {
        console.log('❌ Simple native call failed:', e.message);
    }
} catch (e) {
    console.log('❌ Native module loading failed:', e.message);
}

console.log('\n=== Analysis Complete ===');