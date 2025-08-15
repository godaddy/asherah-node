console.log('Testing synchronous Go function...');

const asherah = require('./build/Release/asherah.node');
console.log('Module loaded, methods:', Object.getOwnPropertyNames(asherah).length);

// Test the synchronous setup function
console.log('Testing setup (sync)...');
try {
    const config = JSON.stringify({
        "ServiceName": "test-service",
        "ProductID": "test-product",
        "Verbose": true,
        "KMS": "static",
        "Metastore": "memory",
        "EnableSessionCaching": false
    });
    
    console.log('Calling setup with config...');
    asherah.setup(config);
    console.log('✅ Setup completed successfully!');
    
    // Try another sync function
    console.log('\nTesting setenv...');
    asherah.setenv('{"TEST": "value"}');
    console.log('✅ Setenv completed successfully!');
    
} catch (e) {
    console.error('❌ Error:', e.message);
    console.error('Stack:', e.stack);
}