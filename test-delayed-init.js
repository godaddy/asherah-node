#!/usr/bin/env bun

const fs = require('fs');

console.log('Testing delayed Asherah initialization...\n');

// Load native module immediately 
let asherah;
try {
    asherah = require('./go_setup_full.node');
    console.log('✅ Native module loaded successfully');
} catch (e) {
    console.error('❌ Failed to load native module:', e.message);
    process.exit(1);
}

const config = JSON.stringify({
    ServiceName: "test",
    ProductID: "test", 
    KMS: "static",
    Metastore: "memory"
});

console.log('\nTesting different execution phases:\n');

// Phase 1: Immediate call (this is what fails)
console.log('Phase 1: Immediate call (sync)');
try {
    const result1 = asherah.setupSync(config);
    console.log('✅ Immediate sync call succeeded:', result1);
} catch (e) {
    console.log('❌ Immediate sync call failed:', e.message);
}

// Phase 2: setTimeout(0) - next tick
setTimeout(() => {
    console.log('\nPhase 2: setTimeout(0) (sync)');
    try {
        const result2 = asherah.setupSync(config);
        console.log('✅ setTimeout(0) sync call succeeded:', result2);
    } catch (e) {
        console.log('❌ setTimeout(0) sync call failed:', e.message);
    }
}, 0);

// Phase 3: setImmediate (if available)
if (typeof setImmediate !== 'undefined') {
    setImmediate(() => {
        console.log('\nPhase 3: setImmediate (sync)');
        try {
            const result3 = asherah.setupSync(config);
            console.log('✅ setImmediate sync call succeeded:', result3);
        } catch (e) {
            console.log('❌ setImmediate sync call failed:', e.message);
        }
    });
}

// Phase 4: After significant delay
setTimeout(() => {
    console.log('\nPhase 4: After 100ms delay (sync)');
    try {
        const result4 = asherah.setupSync(config);
        console.log('✅ Delayed sync call succeeded:', result4);
    } catch (e) {
        console.log('❌ Delayed sync call failed:', e.message);
    }
}, 100);

// Phase 5: After process stabilization
setTimeout(() => {
    console.log('\nPhase 5: After 1000ms delay (sync)');
    try {
        const result5 = asherah.setupSync(config);
        console.log('✅ Very delayed sync call succeeded:', result5);
        console.log('\n=== Test Complete ===');
        process.exit(0);
    } catch (e) {
        console.log('❌ Very delayed sync call failed:', e.message);
        console.log('\n=== Test Complete ===');
        process.exit(1);
    }
}, 1000);

console.log('Scheduled all test phases...\n');