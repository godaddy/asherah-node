console.log('Testing minimal CGO module loading...');

try {
    const asherah = require('./build/Release/asherah.node');
    console.log('✅ Module loaded successfully, methods:', Object.getOwnPropertyNames(asherah).length);
    console.log('Methods:', Object.getOwnPropertyNames(asherah));
} catch (e) {
    console.error('❌ Error:', e.message);
    console.error('Stack:', e.stack);
}

console.log('Test completed.');