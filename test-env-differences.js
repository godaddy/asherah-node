#!/usr/bin/env bun

console.log('=== Environment Analysis: Bun vs Node.js ===\n');

console.log('1. Process information:');
console.log('   Process name:', process.title);
console.log('   Process argv0:', process.argv0);
console.log('   Process platform:', process.platform);
console.log('   Process arch:', process.arch);

console.log('\n2. Memory information:');
const memUsage = process.memoryUsage();
console.log('   RSS:', Math.round(memUsage.rss / 1024 / 1024), 'MB');
console.log('   Heap Used:', Math.round(memUsage.heapUsed / 1024 / 1024), 'MB');
console.log('   Heap Total:', Math.round(memUsage.heapTotal / 1024 / 1024), 'MB');

console.log('\n3. Environment variables (Go-related):');
const goEnvVars = Object.keys(process.env).filter(key => 
    key.startsWith('GO') || key.includes('CGO') || key.includes('MALLOC')
);
goEnvVars.forEach(key => {
    console.log(`   ${key}:`, process.env[key]);
});

console.log('\n4. Signal handlers:');
console.log('   SIGTERM handler:', typeof process.listeners('SIGTERM')[0]);
console.log('   SIGINT handler:', typeof process.listeners('SIGINT')[0]);

console.log('\n5. Thread information:');
if (process.getActiveResourcesInfo) {
    console.log('   Active resources:', process.getActiveResourcesInfo());
}

console.log('\n6. libuv handles:');
if (process._getActiveHandles) {
    console.log('   Active handles:', process._getActiveHandles().length);
}

console.log('\n7. Timing:');
console.log('   Process uptime:', Math.round(process.uptime() * 1000), 'ms');
console.log('   Process hrtime:', process.hrtime());

console.log('\n8. Module loading:');
console.log('   Module paths length:', require.main?.paths?.length || 'N/A');
console.log('   Module filename:', require.main?.filename || 'N/A');