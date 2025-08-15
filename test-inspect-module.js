#!/usr/bin/env bun

console.log('Inspecting native module exports...\n');

try {
    const asherah = require('./go_setup_full.node');
    console.log('Module loaded successfully');
    console.log('Module type:', typeof asherah);
    console.log('Module keys:', Object.keys(asherah));
    console.log('Module properties:');
    
    for (const key of Object.keys(asherah)) {
        console.log(`  ${key}: ${typeof asherah[key]}`);
    }
    
    console.log('\nModule prototype:', Object.getPrototypeOf(asherah));
    console.log('Module constructor:', asherah.constructor?.name);
    
} catch (e) {
    console.error('Failed to load module:', e.message);
}