#!/usr/bin/env bun

console.log('Testing signal handling...');

// Check what signals are being used
process.on('SIGURG', () => {
    console.log('Received SIGURG (Go preemption signal)');
});

process.on('SIGPIPE', () => {
    console.log('Received SIGPIPE');
});

// Test if blocking signals affects Go
const { spawnSync } = require('child_process');

// First test - normal execution
console.log('\nTest 1: Normal execution');
const result1 = spawnSync(process.execPath, ['-e', `
    const asherah = require('${process.cwd()}/build/Release/asherah.node');
    console.log('Module loaded');
    setTimeout(() => {
        console.log('Timeout - hanging');
        process.exit(1);
    }, 500);
    asherah.setenv('{"test":"value"}');
    console.log('Success');
`], { timeout: 1000 });

console.log('Exit code:', result1.status);
console.log('Output:', result1.stdout?.toString());
console.log('Error:', result1.stderr?.toString());

// Test with GODEBUG to disable async preemption
console.log('\nTest 2: With GODEBUG=asyncpreemptoff=1');
const result2 = spawnSync(process.execPath, ['-e', `
    const asherah = require('${process.cwd()}/build/Release/asherah.node');
    console.log('Module loaded with preemption off');
    setTimeout(() => {
        console.log('Timeout - hanging');
        process.exit(1);
    }, 500);
    asherah.setenv('{"test":"value"}');
    console.log('Success');
`], { 
    timeout: 1000,
    env: { ...process.env, GODEBUG: 'asyncpreemptoff=1' }
});

console.log('Exit code:', result2.status);
console.log('Output:', result2.stdout?.toString());
console.log('Error:', result2.stderr?.toString());