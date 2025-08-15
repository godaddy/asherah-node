/**
 * Bun compatibility integration test
 * 
 * This test ensures that asherah-node works correctly in Bun runtime
 * with the Go warmup library initialization.
 * 
 * To run this test with Bun:
 *   bun test test/integration/bun-compatibility.spec.js
 */

const { expect } = require('chai');
const asherah = require('../../src/index');

describe('Bun Runtime Compatibility', function() {
  this.timeout(10000);

  const config = {
    KMS: 'static',
    Metastore: 'memory',
    ServiceName: 'bun-test-service',
    ProductID: 'bun-test-product',
    Verbose: false,
    EnableSessionCaching: false,
    ExpireAfter: 60,
    CheckInterval: 1
  };


  it('should successfully setup in Bun runtime', function() {
    asherah.setup(config);
    // If we get here without crashing, setup succeeded
    expect(true).to.be.true;
  });

  it('should encrypt and decrypt data correctly', function() {
    const partitionId = 'test-partition-' + Date.now();
    const originalData = 'Test data for Bun compatibility: 🚀';
    
    // Encrypt
    const encrypted = asherah.encrypt(partitionId, Buffer.from(originalData, 'utf8'));
    expect(encrypted).to.not.be.null;
    expect(encrypted).to.be.a('string');
    expect(encrypted.length).to.be.greaterThan(0);
    
    // Decrypt
    const decryptedBuffer = asherah.decrypt(partitionId, encrypted);
    expect(decryptedBuffer).to.not.be.null;
    
    const decryptedData = decryptedBuffer.toString('utf8');
    expect(decryptedData).to.equal(originalData);
  });

  it('should handle multiple encrypt/decrypt cycles', function() {
    const partitionId = 'test-partition-multi-' + Date.now();
    const testData = [
      'First test message',
      'Second test with special chars: €£¥',
      'Third test with emoji: 🎉🔐🔑',
      JSON.stringify({ test: 'object', number: 42 })
    ];
    
    testData.forEach((data, index) => {
      // Encrypt
      const encrypted = asherah.encrypt(partitionId, Buffer.from(data, 'utf8'));
      expect(encrypted).to.not.be.null;
      expect(encrypted).to.be.a('string');
      
      // Decrypt
      const decrypted = asherah.decrypt(partitionId, encrypted);
      expect(decrypted).to.not.be.null;
      expect(decrypted.toString('utf8')).to.equal(data);
    });
  });

  it('should successfully shutdown', function() {
    asherah.shutdown();
    // If we get here without crashing, shutdown succeeded
    expect(true).to.be.true;
  });

  it('should handle full lifecycle: setup, encrypt, decrypt, shutdown', function() {
    // Setup
    asherah.setup(config);
    
    // Encrypt
    const partitionId = 'test-lifecycle-' + Date.now();
    const testData = 'Full lifecycle test in Bun';
    const encrypted = asherah.encrypt(partitionId, Buffer.from(testData, 'utf8'));
    expect(encrypted).to.not.be.null;
    
    // Decrypt
    const decrypted = asherah.decrypt(partitionId, encrypted);
    expect(decrypted.toString('utf8')).to.equal(testData);
    
    // Shutdown
    asherah.shutdown();
    
    // All operations completed successfully
    expect(true).to.be.true;
  });
});
