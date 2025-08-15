#!/usr/bin/env bun

const { dlopen, FFIType } = require('bun:ffi');

console.log('Testing Go complexity levels in Bun...\n');

const lib = dlopen('./test-complexity.dylib', {
    Level1Simple: { returns: FFIType.int, args: [] },
    Level2Fmt: { returns: FFIType.int, args: [] },
    Level3Json: { returns: FFIType.int, args: [] },
    Level4Crypto: { returns: FFIType.int, args: [] },
    Level5Time: { returns: FFIType.int, args: [] },
    Level6Memory: { returns: FFIType.int, args: [] },
});

const levels = [
    { name: 'Level1Simple', desc: 'Ultra simple' },
    { name: 'Level2Fmt', desc: 'With fmt' },
    { name: 'Level3Json', desc: 'With JSON' },
    { name: 'Level4Crypto', desc: 'With crypto' },
    { name: 'Level5Time', desc: 'With time' },
    { name: 'Level6Memory', desc: 'With memory' },
];

for (const level of levels) {
    try {
        console.log(`Testing ${level.name}: ${level.desc}`);
        const result = lib.symbols[level.name]();
        console.log(`✅ ${level.name} succeeded: ${result}`);
    } catch (e) {
        console.error(`❌ ${level.name} failed: ${e.message}`);
        break;
    }
}

console.log('\nComplexity test complete!');