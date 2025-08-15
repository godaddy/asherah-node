console.log('Testing asynchronous Go function...');

const asherah = require('./build/Release/asherah.node');
console.log('Module loaded, methods:', Object.getOwnPropertyNames(asherah).length);

// Test the asynchronous setup function
console.log('Testing setup_async...');
try {
    const config = JSON.stringify({
        "ServiceName": "test-service",
        "ProductID": "test-product",
        "Verbose": true,
        "KMS": {
            "Type": "static",
            "Config": {}
        },
        "Metastore": {
            "Type": "memory"
        }
    });
    
    console.log('Calling setup_async with config...');
    asherah.setup_async(config, (err, result) => {
        if (err) {
            console.error('❌ Async Error:', err.message);
        } else {
            console.log('✅ Async setup completed successfully!', result);
        }
        console.log('Async test completed.');
    });
    
    console.log('setup_async call initiated, waiting for callback...');
    
} catch (e) {
    console.error('❌ Error:', e.message);
    console.error('Stack:', e.stack);
}