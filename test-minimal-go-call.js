#!/usr/bin/env node

// Minimal test to see if we can call ANY Go function from Bun
console.log('Testing minimal Go function call with Bun...');
console.log('Runtime:', typeof Bun !== 'undefined' ? 'Bun' : 'Node.js');

const asherah = require('./build/Release/asherah.node');
console.log('Module loaded successfully');

// Try calling a simple function that should return quickly
try {
  console.log('\nTesting SetEnv (simpler Go function)...');
  const startTime = Date.now();
  
  // SetEnv is a simpler Go function that just sets environment variables
  asherah.setenv(JSON.stringify({}));
  
  const elapsed = Date.now() - startTime;
  console.log(`✅ setenv completed in ${elapsed}ms`);
  
  process.exit(0);
} catch (error) {
  console.error('❌ ERROR:', error.message);
  console.error(error.stack);
  process.exit(1);
}