#!/usr/bin/env node

console.log('Testing simple module load...');
console.log('Runtime:', process.versions.bun ? 'Bun' : 'Node.js');

try {
    console.log('Loading asherah module...');
    const asherah = require('./build/Release/asherah.node');
    console.log('✅ Module loaded successfully');
    
    console.log('Testing simple function call...');
    const config = JSON.stringify({"ProductID": "test", "ServiceName": "test", "Verbose": true});
    asherah.setenv(config);
    console.log('✅ setenv call completed');
    
} catch (e) {
    console.error('❌ Error:', e.message);
    process.exit(1);
}