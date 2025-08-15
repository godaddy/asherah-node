console.log('Testing simple CGO function call...');

try {
    const asherah = require('./build/Release/asherah.node');
    console.log('✅ Module loaded successfully');
    
    console.log('Calling get_setup_status...');
    const status = asherah.get_setup_status();
    console.log('✅ get_setup_status returned:', status);
    
} catch (e) {
    console.error('❌ Error:', e.message);
    console.error('Stack:', e.stack);
}

console.log('Test completed.');