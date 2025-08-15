#!/usr/bin/env bun

console.log('Testing optimized C library implementation...');

try {
    console.log('1. Loading asherah-node with optimized C warmup...');
    process.env.ASHERAH_BUN_VERBOSE = 'true';
    const asherah = require('./src/index.js');
    console.log('✅ Module loaded successfully');
    
} catch (error) {
    console.error('❌ Module load failed:', error.message);
}