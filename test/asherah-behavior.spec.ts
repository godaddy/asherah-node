import { describe, it, beforeEach, afterEach } from 'mocha';
import { strict as assert } from 'assert';
import {
    asherah_setup_static_memory_async,
    asherah_shutdown_async,
    assert_throws_async
} from './asherah';
import {
    setup_async,
    encrypt_async,
    decrypt_async,
    encrypt_string,
    encrypt_string_async,
    decrypt_string,
    decrypt_string_async,
    get_setup_status,
    set_max_stack_alloc_item_size
} from '../dist/asherah';

describe('Asherah Behavior Tests', function() {
    this.timeout(5000); // Allow longer timeout for stress tests

    describe('Setup and Shutdown Behavior', function() {
        it('should not allow double setup', async function() {
            await asherah_setup_static_memory_async();
            try {
                await assert_throws_async(async () => {
                    await setup_async({
                        ServiceName: 'test',
                        ProductID: 'test',
                        Metastore: 'memory',
                        KMS: 'static',
                        ExpireAfter: null,
                        CheckInterval: null,
                        ConnectionString: null,
                        ReplicaReadConsistency: null,
                        DynamoDBEndpoint: null,
                        DynamoDBRegion: null,
                        DynamoDBTableName: null,
                        SessionCacheMaxSize: null,
                        SessionCacheDuration: null,
                        RegionMap: null,
                        PreferredRegion: null,
                        EnableRegionSuffix: null,
                        EnableSessionCaching: null,
                        Verbose: null
                    });
                }, 'Should not allow setup when already initialized');
            } finally {
                await asherah_shutdown_async();
            }
        });

        it('should not allow operations after shutdown', async function() {
            await asherah_setup_static_memory_async();
            await asherah_shutdown_async();
            
            await assert_throws_async(async () => {
                await encrypt_string_async('partition', 'data');
            }, 'Should not allow encryption after shutdown');
        });

        it('should handle rapid setup/shutdown cycles', async function() {
            for (let i = 0; i < 10; i++) {
                await asherah_setup_static_memory_async();
                assert(get_setup_status(), `Setup should succeed on iteration ${i}`);
                await asherah_shutdown_async();
                assert(!get_setup_status(), `Shutdown should succeed on iteration ${i}`);
            }
        });
    });

    describe('Encryption/Decryption Behavior', function() {
        beforeEach(async function() {
            await asherah_setup_static_memory_async();
        });

        afterEach(async function() {
            await asherah_shutdown_async();
        });

        it('should handle maximum size data', async function() {
            // Test with progressively larger data sizes
            const sizes = [1024, 10240, 102400, 1048576]; // 1KB, 10KB, 100KB, 1MB
            
            for (const size of sizes) {
                const largeData = Buffer.alloc(size, 'x');
                const encrypted = await encrypt_async('partition', largeData);
                const decrypted = await decrypt_async('partition', encrypted);
                
                assert(Buffer.compare(largeData, decrypted) === 0, 
                    `Should correctly handle ${size} byte buffer`);
            }
        });

        it('should handle unicode and special characters', async function() {
            const testStrings = [
                '你好世界', // Chinese
                '🔐🔑🛡️', // Emojis
                'Hello\x00World', // Null byte
                'Line1\nLine2\rLine3\r\n', // Various line endings
                '{"json": "value", "nested": {"key": "value"}}', // JSON
                '<script>alert("xss")</script>', // HTML/JS
                'Robert\'); DROP TABLE Students;--', // SQL injection attempt
                '\\u0000\\u0001\\u0002', // Unicode escapes
            ];

            for (const testStr of testStrings) {
                const encrypted = await encrypt_string_async('partition', testStr);
                const decrypted = await decrypt_string_async('partition', encrypted);
                assert.strictEqual(decrypted, testStr, 
                    `Should handle special string: ${testStr.substring(0, 20)}...`);
            }
        });

        it('should produce different ciphertexts for same plaintext', async function() {
            const plaintext = 'same data';
            const encrypted1 = await encrypt_string_async('partition', plaintext);
            const encrypted2 = await encrypt_string_async('partition', plaintext);
            
            assert.notStrictEqual(encrypted1, encrypted2, 
                'Should produce different ciphertexts (nonce/IV randomization)');
            
            // But both should decrypt to same value
            const decrypted1 = await decrypt_string_async('partition', encrypted1);
            const decrypted2 = await decrypt_string_async('partition', encrypted2);
            assert.strictEqual(decrypted1, plaintext);
            assert.strictEqual(decrypted2, plaintext);
        });

        it('should handle concurrent operations', async function() {
            const operations = 100;
            const promises = [];
            
            // Mix of encryptions and decryptions
            for (let i = 0; i < operations; i++) {
                if (i % 2 === 0) {
                    promises.push(
                        encrypt_string_async(`partition${i % 10}`, `data${i}`)
                            .then(encrypted => decrypt_string_async(`partition${i % 10}`, encrypted))
                            .then(decrypted => ({ original: `data${i}`, decrypted }))
                    );
                } else {
                    promises.push(
                        encrypt_async(`partition${i % 10}`, Buffer.from(`data${i}`))
                            .then(encrypted => decrypt_async(`partition${i % 10}`, encrypted))
                            .then(decrypted => ({ 
                                original: `data${i}`, 
                                decrypted: decrypted.toString() 
                            }))
                    );
                }
            }
            
            const results = await Promise.all(promises);
            results.forEach(({ original, decrypted }) => {
                assert.strictEqual(decrypted, original, 
                    'Concurrent operations should maintain data integrity');
            });
        });

        it('should handle partition isolation', async function() {
            const data = 'secret data';
            const encrypted1 = await encrypt_string_async('partition1', data);
            const encrypted2 = await encrypt_string_async('partition2', data);
            
            // Encrypted data should be different for different partitions
            assert.notStrictEqual(encrypted1, encrypted2, 
                'Different partitions should produce different ciphertexts');
            
            // Should not be able to decrypt with wrong partition
            await assert_throws_async(async () => {
                await decrypt_string_async('partition2', encrypted1);
            }, 'Should not decrypt data from different partition');
        });
    });

    describe('Error Handling Behavior', function() {
        beforeEach(async function() {
            await asherah_setup_static_memory_async();
        });

        afterEach(async function() {
            await asherah_shutdown_async();
        });

        it('should handle invalid encrypted data gracefully', async function() {
            const invalidInputs = [
                'not-json',
                '{}', // Valid JSON but not valid envelope
                '{"wrong": "format"}',
                JSON.stringify({ ParentKeyMeta: {} }), // Missing required fields
                '', // Empty string already tested elsewhere
                'null',
                'undefined',
                '[1,2,3]', // Array instead of object
            ];

            for (const invalid of invalidInputs) {
                await assert_throws_async(async () => {
                    await decrypt_string_async('partition', invalid);
                }, `Should reject invalid encrypted data: ${invalid}`);
            }
        });

        it('should handle buffer/string type mismatches', async function() {
            // Encrypt as string, try to decrypt as buffer
            const stringEncrypted = await encrypt_string_async('partition', 'test');
            const bufferResult = await decrypt_async('partition', stringEncrypted);
            assert(Buffer.isBuffer(bufferResult), 'Should return buffer from decrypt');
            assert.strictEqual(bufferResult.toString(), 'test', 'Should decrypt correctly');

            // Encrypt as buffer, try to decrypt as string
            const bufferEncrypted = await encrypt_async('partition', Buffer.from('test'));
            const stringResult = await decrypt_string_async('partition', bufferEncrypted);
            assert.strictEqual(typeof stringResult, 'string', 'Should return string from decrypt_string');
            assert.strictEqual(stringResult, 'test', 'Should decrypt correctly');
        });
    });

    describe('Stack Allocation Behavior', function() {
        it('should handle different stack allocation sizes', async function() {
            const testSizes = [0, 1, 1024, 4096, 65536];
            
            for (const size of testSizes) {
                await asherah_setup_static_memory_async(false, true, size);
                
                try {
                    // Test with data that might use stack allocation
                    const data = 'x'.repeat(Math.min(size / 2, 100));
                    const encrypted = await encrypt_string_async('partition', data);
                    const decrypted = await decrypt_string_async('partition', encrypted);
                    assert.strictEqual(decrypted, data, 
                        `Should work with stack size ${size}`);
                } finally {
                    await asherah_shutdown_async();
                }
            }
        });

        it('should handle negative stack allocation size', async function() {
            await asherah_setup_static_memory_async();
            
            // Should clamp negative values to 0 (force heap allocation)
            set_max_stack_alloc_item_size(-1);
            
            const data = 'test data';
            const encrypted = await encrypt_string_async('partition', data);
            const decrypted = await decrypt_string_async('partition', encrypted);
            assert.strictEqual(decrypted, data, 
                'Should handle negative stack size by using heap');
            
            await asherah_shutdown_async();
        });

        it('should handle very large stack allocation size', async function() {
            await asherah_setup_static_memory_async();
            
            // Should clamp to reasonable maximum (1MB)
            set_max_stack_alloc_item_size(2147483647); // INT32_MAX
            
            const data = 'test data';
            const encrypted = await encrypt_string_async('partition', data);
            const decrypted = await decrypt_string_async('partition', encrypted);
            assert.strictEqual(decrypted, data, 
                'Should handle very large stack size request');
            
            await asherah_shutdown_async();
        });
    });

    describe('Sync vs Async Behavior Consistency', function() {
        beforeEach(async function() {
            await asherah_setup_static_memory_async();
        });

        afterEach(async function() {
            await asherah_shutdown_async();
        });

        it('should produce compatible results between sync and async', async function() {
            const testData = 'test data for compatibility';
            
            // Encrypt sync, decrypt async
            const encryptedSync = encrypt_string('partition', testData);
            const decryptedAsync = await decrypt_string_async('partition', encryptedSync);
            assert.strictEqual(decryptedAsync, testData, 
                'Sync encrypted data should decrypt correctly with async');

            // Encrypt async, decrypt sync
            const encryptedAsync = await encrypt_string_async('partition', testData);
            const decryptedSync = decrypt_string('partition', encryptedAsync);
            assert.strictEqual(decryptedSync, testData, 
                'Async encrypted data should decrypt correctly with sync');
        });

        it('should handle errors consistently between sync and async', function() {
            // Both should throw for empty partition - just verify they throw, not the exact message
            assert.throws(() => {
                encrypt_string('', 'data');
            }, 'Sync should throw for empty partition');

            return assert_throws_async(async () => {
                await encrypt_string_async('', 'data');
            }, 'Async should throw for empty partition');
        });
    });
});