console.log('Testing direct CGO function call with minimal setup...');

try {
    const asherah = require('./build/Release/asherah.node');
    console.log('Module loaded successfully');
    
    // Test the simplest possible CGO function
    console.log('Calling setenv with minimal data...');
    asherah.setenv('{}');
    console.log('✅ setenv succeeded');
    
} catch (e) {
    console.error('❌ Error:', e.message);
    console.error('Stack:', e.stack);
}

console.log('Test completed.');
