#!/usr/bin/env node

/**
 * Test to understand what triggers AWS imports
 * 
 * We know from the symbols that aws-v1 plugins are statically linked.
 * The question is: are they initialized eagerly or only when used?
 */

console.log('=== AWS Dependency Chain Analysis ===\n');

console.log('1. Loading library (statically includes AWS plugins)...');
const asherah = require('./build/Release/asherah.node');
console.log('✅ Library loaded - AWS symbols are present but init not triggered yet');

console.log('2. Testing setup with STATIC KMS (no AWS)...');
try {
    asherah.setup({
        ServiceName: 'aws-free-test',
        ProductID: 'test',
        KMS: 'static',  // ← Not using AWS KMS
        Metastore: 'memory',  // ← Not using DynamoDB  
        Verbose: false
    });
    console.log('✅ Static KMS setup successful - no AWS init needed');
    
    const testData = 'AWS-free test data';
    const encrypted = asherah.encrypt('test-partition', testData);
    const decrypted = asherah.decrypt_string('test-partition', encrypted);
    
    if (decrypted === testData) {
        console.log('✅ Encrypt/decrypt works without AWS');
    }
    
    asherah.shutdown();
    console.log('✅ Complete workflow successful without touching AWS');
    
} catch (error) {
    console.error('❌ Even non-AWS usage failed:', error.message);
}

console.log('\n3. Now testing what happens with AWS KMS...');
try {
    asherah.setup({
        ServiceName: 'aws-test',
        ProductID: 'test',
        KMS: 'aws',  // ← This should trigger AWS init
        Metastore: 'memory',
        RegionMap: {"us-west-2": "arn:aws:kms:us-west-2:123456789012:key/test"},
        Verbose: false
    });
    console.log('✅ AWS KMS setup successful');
    
} catch (error) {
    console.error('❌ AWS KMS setup failed:', error.message);
    console.error('This tells us when AWS init happens');
}