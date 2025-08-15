#!/usr/bin/env bun

console.log('Testing self-warming module (loaded FIRST)...\n');

// This module should warm itself up during Init
console.log('Loading self-warming module...');
const selfWarmup = require('./test-self-warmup.node');

console.log('Module loaded, exports:', Object.keys(selfWarmup));

console.log('\nCalling setupSync...');
const result = selfWarmup.setupSync();
console.log('✅ setupSync returned:', result);

console.log('\n=== SUCCESS: Self-warming module works! ===');