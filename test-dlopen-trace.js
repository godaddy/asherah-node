#!/usr/bin/env node

// Test to trace where the hang occurs
console.log('Testing dlopen trace with Bun...');
console.log('Runtime:', typeof Bun !== 'undefined' ? 'Bun' : 'Node.js');

// Use process.dlopen directly to load the module
const module = { exports: {} };
const filename = require('path').join(__dirname, 'build/Release/asherah.node');

console.log('About to call process.dlopen...');
console.log('Filename:', filename);

try {
  const startTime = Date.now();
  
  // This is what require() does internally
  process.dlopen(module, filename);
  
  const elapsed = Date.now() - startTime;
  console.log(`✅ dlopen completed in ${elapsed}ms`);
  console.log('Module exports:', Object.keys(module.exports));
  
  process.exit(0);
} catch (error) {
  console.error('❌ ERROR:', error.message);
  console.error(error.stack);
  process.exit(1);
}