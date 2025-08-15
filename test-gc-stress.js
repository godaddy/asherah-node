#!/usr/bin/env node

console.log('Testing GC behavior with handle scope changes...');
console.log('Runtime:', process.versions.bun ? 'Bun' : 'Node.js');

// Create memory pressure to trigger GC
function createMemoryPressure() {
    const arrays = [];
    for (let i = 0; i < 1000; i++) {
        arrays.push(new Array(1000).fill(Math.random()));
    }
    return arrays.length;
}

// Test native module under GC pressure
async function testUnderGCPressure() {
    console.log('Creating memory pressure to trigger GC...');
    
    for (let round = 0; round < 5; round++) {
        console.log(`Round ${round + 1}:`);
        
        // Create memory pressure
        const count = createMemoryPressure();
        console.log(`  Created ${count} arrays`);
        
        // Force GC if available
        if (global.gc) {
            global.gc();
            console.log('  Manual GC triggered');
        }
        
        // Test native module operations during/after GC
        try {
            const crypto = require('crypto');
            const hash = crypto.createHash('sha256').update(`test${round}`).digest('hex');
            console.log(`  ✅ Crypto works: ${hash.substring(0, 8)}...`);
        } catch (e) {
            console.error(`  ❌ Crypto failed: ${e.message}`);
        }
        
        // Test async operation
        try {
            await new Promise(resolve => setTimeout(resolve, 10));
            console.log('  ✅ Async operation works');
        } catch (e) {
            console.error(`  ❌ Async failed: ${e.message}`);
        }
    }
    
    console.log('GC stress test completed');
}

testUnderGCPressure();