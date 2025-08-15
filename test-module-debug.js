console.log('Testing module initialization...');

try {
    console.log('About to require module...');
    const asherah = require('./build/Release/asherah.node');
    console.log('Module loaded, type:', typeof asherah);
    console.log('Module:', asherah);
    console.log('Keys:', Object.keys(asherah));
    console.log('Own property names:', Object.getOwnPropertyNames(asherah));
    console.log('Own property symbols:', Object.getOwnPropertySymbols(asherah));
    
    // Try to access known methods directly
    console.log('setup:', asherah.setup);
    console.log('setenv:', asherah.setenv);
} catch (e) {
    console.error('Error:', e);
}