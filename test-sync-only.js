#!/usr/bin/env bun

console.log('Testing SYNC only...');

const addon = require('./go_setup_full.node');

console.log('About to call setupSync...');
try {
    const result = addon.setupSync();
    console.log('✅ Sync result:', result);
} catch (e) {
    console.error('❌ Error:', e.message);
}
