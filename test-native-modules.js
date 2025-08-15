#!/usr/bin/env node

console.log('Testing various native modules...');
console.log('Runtime:', process.versions.bun ? 'Bun' : 'Node.js');

// Test 1: Built-in native modules
try {
    const crypto = require('crypto');
    const hash = crypto.createHash('sha256').update('test').digest('hex');
    console.log('✅ crypto module works:', hash.substring(0, 8) + '...');
} catch (e) {
    console.error('❌ crypto module failed:', e.message);
}

// Test 2: OS operations
try {
    const os = require('os');
    console.log('✅ os module works, platform:', os.platform());
} catch (e) {
    console.error('❌ os module failed:', e.message);
}

// Test 3: File system
try {
    const fs = require('fs');
    const exists = fs.existsSync(__filename);
    console.log('✅ fs module works, file exists:', exists);
} catch (e) {
    console.error('❌ fs module failed:', e.message);
}

// Test 4: Path operations
try {
    const path = require('path');
    const parsed = path.parse(__filename);
    console.log('✅ path module works, extension:', parsed.ext);
} catch (e) {
    console.error('❌ path module failed:', e.message);
}