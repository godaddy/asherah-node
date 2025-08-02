


import {
    asherah_setup,
    asherah_setup_static_memory,
    asherah_setup_static_memory_async,
    asherah_shutdown,
    asherah_shutdown_async,
    AsherahConfig,
    assert_asherah_shutdown,
    assert_throws_async,
    test_round_trip_buffers,
    test_round_trip_buffers_async,
    test_round_trip_strings,
    test_round_trip_strings_async
} from './asherah'
import { get_string } from './helpers';
import { strict as assert } from 'assert';

const force_use_heap = 0;
const test_verbose = true;
const default_max_stack_alloc_item_size = 2048;
const big_string_size = 65535;
const simple_secret = 'mysecretdata';

describe('Asherah', function () {
    it('Bad No Setup Async', async function () {
        await bad_no_setup_async();
    });

    it('Bad Encrypt String Undefined', async function () {
        await bad_encrypt_string_undefined_async(test_verbose, true, default_max_stack_alloc_item_size);
    });

    it('Bad Encrypt String Null', async function () {
        await bad_encrypt_string_null_async(test_verbose, true, default_max_stack_alloc_item_size);
    });

    it('Bad Encrypt String Empty', async function () {
        await bad_encrypt_string_empty_async(test_verbose, true, default_max_stack_alloc_item_size);
    });

    it('Bad Encrypt Buffers Undefined', async function () {
        await bad_encrypt_buffers_undefined_async(test_verbose, true, default_max_stack_alloc_item_size);
    });

    it('Bad Encrypt Buffers Null', async function () {
        await bad_encrypt_buffers_null_async(test_verbose, true, default_max_stack_alloc_item_size);
    });

    it('Bad Encrypt Buffers Empty', async function () {
        await bad_encrypt_buffers_empty_async(test_verbose, true, default_max_stack_alloc_item_size);
    });

    it('Bad Empty Partition ID Sync', async function () {
        await bad_empty_partition_id_sync(test_verbose, true, default_max_stack_alloc_item_size);
    });

    it('Bad Empty Partition ID Async', async function () {
        await bad_empty_partition_id_async(test_verbose, true, default_max_stack_alloc_item_size);
    });

    it('Bad Empty Partition ID Decrypt Sync', async function () {
        await bad_empty_partition_id_decrypt_sync(test_verbose, true, default_max_stack_alloc_item_size);
    });

    it('Bad Empty Partition ID Decrypt Async', async function () {
        await bad_empty_partition_id_decrypt_async(test_verbose, true, default_max_stack_alloc_item_size);
    });

    // Test using heap only
    register_sync_roundtrip_tests(simple_secret, test_verbose, false, force_use_heap);
    register_sync_roundtrip_tests(simple_secret, test_verbose, true, force_use_heap);
    register_sync_roundtrip_tests(get_string(big_string_size), test_verbose, false, force_use_heap);
    register_sync_roundtrip_tests(get_string(big_string_size), test_verbose, true, force_use_heap);

    // Test using stack
    register_sync_roundtrip_tests(simple_secret, test_verbose, false, simple_secret.length * 4);
    register_sync_roundtrip_tests(simple_secret, test_verbose, true, simple_secret.length * 4);
    register_sync_roundtrip_tests(get_string(big_string_size), test_verbose, false, big_string_size + 1);
    register_sync_roundtrip_tests(get_string(big_string_size), test_verbose, true, big_string_size + 1);

    // Test async using heap only
    register_async_roundtrip_tests(simple_secret, test_verbose, false, force_use_heap);
    register_async_roundtrip_tests(simple_secret, test_verbose, true, force_use_heap);
    register_async_roundtrip_tests(get_string(big_string_size), test_verbose, false, force_use_heap);
    register_async_roundtrip_tests(get_string(big_string_size), test_verbose, true, force_use_heap);

    // Test async using stack
    register_async_roundtrip_tests(simple_secret, test_verbose, false, simple_secret.length * 4);
    register_async_roundtrip_tests(simple_secret, test_verbose, true, simple_secret.length * 4);
    register_async_roundtrip_tests(get_string(big_string_size), test_verbose, false, big_string_size + 1);
    register_async_roundtrip_tests(get_string(big_string_size), test_verbose, true, big_string_size + 1);

    it('Test RegionMap', function () {
        const config: AsherahConfig = {
            KMS: 'aws',
            Metastore: 'test-debug-memory',
            ServiceName: 'TestService',
            ProductID: 'TestProduct',
            Verbose: true,
            EnableSessionCaching: false,
            ExpireAfter: null,
            CheckInterval: null,
            ConnectionString: null,
            ReplicaReadConsistency: null,
            DynamoDBEndpoint: null,
            DynamoDBRegion: null,
            DynamoDBTableName: null,
            SessionCacheMaxSize: null,
            SessionCacheDuration: null,
            RegionMap: { "us-west-2": "arn:aws:kms:us-west-2:795066905288:key/3a628d06-9db4-4b1f-9f76-54fc742dc662" },
            PreferredRegion: null,
            EnableRegionSuffix: null
        };
        asherah_setup(config);
        asherah_shutdown();
    });
});

function register_sync_roundtrip_tests(secret_data: string, verbose: boolean, session_cache: boolean, max_stack_alloc_item_size: number) {
    it(get_test_name('Round Trip Buffers Sync', verbose, session_cache, max_stack_alloc_item_size), function () {
        roundtrip_buffer(secret_data, verbose, session_cache, max_stack_alloc_item_size);
    });

    it(get_test_name('Round Trip Strings Sync', verbose, session_cache, max_stack_alloc_item_size), function () {
        roundtrip_string(secret_data, verbose, session_cache, max_stack_alloc_item_size);
    });
}

function register_async_roundtrip_tests(secret_data: string, verbose: boolean, session_cache: boolean, max_stack_alloc_item_size: number) {
    it(get_test_name('Round Trip Buffers Async', verbose, session_cache, max_stack_alloc_item_size), async function () {
        await roundtrip_buffer_async(secret_data, verbose, session_cache, max_stack_alloc_item_size);
    });

    it(get_test_name('Round Trip Strings Async', verbose, session_cache, max_stack_alloc_item_size), async function () {
        await roundtrip_string_async(secret_data, verbose, session_cache, max_stack_alloc_item_size);
    });
}

function roundtrip_buffer(secret_data: string, verbose: boolean, session_cache: boolean, max_stack_alloc_item_size: number) {
    asherah_setup_static_memory(verbose, session_cache, max_stack_alloc_item_size);
    try {
        const data = Buffer.from(secret_data, 'utf8');
        test_round_trip_buffers(data);
    } finally {
        asherah_shutdown();
    }
    assert_asherah_shutdown();
}

function roundtrip_string(secret_data: string, verbose: boolean, session_cache: boolean, max_stack_alloc_item_size: number) {
    asherah_setup_static_memory(verbose, session_cache, max_stack_alloc_item_size);
    try {
        test_round_trip_strings(secret_data);
    } finally {
        asherah_shutdown();
    }
    assert_asherah_shutdown();
}

async function roundtrip_buffer_async(secret_data: string, verbose: boolean, session_cache: boolean, max_stack_alloc_item_size: number): Promise<void> {
    await asherah_setup_static_memory_async(verbose, session_cache, max_stack_alloc_item_size);
    try {
        const data = Buffer.from(secret_data, 'utf8');
        await test_round_trip_buffers_async(data);
    } finally {
        await asherah_shutdown_async();
    }
    assert_asherah_shutdown();
}

async function roundtrip_string_async(secret_data: string, verbose: boolean, session_cache: boolean, max_stack_alloc_item_size: number): Promise<void> {
    await asherah_setup_static_memory_async(verbose, session_cache, max_stack_alloc_item_size);
    try {
        await test_round_trip_strings_async(secret_data);
    } finally {
        await asherah_shutdown_async();
    }
    assert_asherah_shutdown();
}

async function bad_no_setup_async(): Promise<void> {
    assert_throws_async(async () => {
        await test_round_trip_buffers_async(Buffer.from('bad'));
    }, 'Should throw error if no setup');
}

async function bad_encrypt_string_undefined_async(verbose: boolean, session_cache: boolean, max_stack_alloc_item_size: number): Promise<void> {
    await asherah_setup_static_memory_async(verbose, session_cache, max_stack_alloc_item_size);
    try {
        assert_throws_async(async () => {
            await test_round_trip_strings(undefined as any);
        }, 'Should throw error if encrypt undefined');
    } finally {
        await asherah_shutdown_async();
    }
}

async function bad_encrypt_string_null_async(verbose: boolean, session_cache: boolean, max_stack_alloc_item_size: number): Promise<void> {
    await asherah_setup_static_memory_async(verbose, session_cache, max_stack_alloc_item_size);
    try {
        assert_throws_async(async () => {
            await test_round_trip_strings_async(null as any);
        }, 'Should throw error if encrypt string null');
    } finally {
        await asherah_shutdown_async();
    }
}

async function bad_encrypt_string_empty_async(verbose: boolean, session_cache: boolean, max_stack_alloc_item_size: number): Promise<void> {
    await asherah_setup_static_memory_async(verbose, session_cache, max_stack_alloc_item_size);
    try {
        assert_throws_async(async () => {
            await test_round_trip_strings_async('');
        }, 'Should throw error if encrypt string empty');
    } finally {
        await asherah_shutdown_async();
    }
}

async function bad_encrypt_buffers_undefined_async(verbose: boolean, session_cache: boolean, max_stack_alloc_item_size: number): Promise<void> {
    await asherah_setup_static_memory_async(verbose, session_cache, max_stack_alloc_item_size);
    try {
        assert_throws_async(async () => {
            await test_round_trip_buffers_async(undefined as any);
        }, 'Should throw error if encrypt buffer undefined');
    } finally {
        await asherah_shutdown_async();
    }
}

async function bad_encrypt_buffers_null_async(verbose: boolean, session_cache: boolean, max_stack_alloc_item_size: number): Promise<void> {
    await asherah_setup_static_memory_async(verbose, session_cache, max_stack_alloc_item_size);
    try {
        assert_throws_async(async () => {
            await test_round_trip_buffers_async(null as any);
        }, 'Should throw error if encrypt buffer null');
    } finally {
        await asherah_shutdown_async();
    }
}

async function bad_encrypt_buffers_empty_async(verbose: boolean, session_cache: boolean, max_stack_alloc_item_size: number): Promise<void> {
    await asherah_setup_static_memory_async(verbose, session_cache, max_stack_alloc_item_size);
    try {
        assert_throws_async(async () => {
            await test_round_trip_buffers_async(Buffer.from(''));
        }, 'Should throw error if encrypt buffer empty');
    } finally {
        await asherah_shutdown_async();
    }
}

async function bad_empty_partition_id_sync(verbose: boolean, session_cache: boolean, max_stack_alloc_item_size: number): Promise<void> {
    await asherah_setup_static_memory_async(verbose, session_cache, max_stack_alloc_item_size);
    try {
        const { encrypt_string } = await import('../dist/asherah');
        try {
            encrypt_string('', 'test data');
            assert(false, 'Should throw error if partition ID is empty');
        } catch (e) {
            // Expected
        }
    } finally {
        await asherah_shutdown_async();
    }
}

async function bad_empty_partition_id_async(verbose: boolean, session_cache: boolean, max_stack_alloc_item_size: number): Promise<void> {
    await asherah_setup_static_memory_async(verbose, session_cache, max_stack_alloc_item_size);
    try {
        const { encrypt_string_async } = await import('../dist/asherah');
        await assert_throws_async(async () => {
            await encrypt_string_async('', 'test data');
        }, 'Should throw error if partition ID is empty');
    } finally {
        await asherah_shutdown_async();
    }
}

async function bad_empty_partition_id_decrypt_sync(verbose: boolean, session_cache: boolean, max_stack_alloc_item_size: number): Promise<void> {
    await asherah_setup_static_memory_async(verbose, session_cache, max_stack_alloc_item_size);
    try {
        const { decrypt_string } = await import('../dist/asherah');
        try {
            decrypt_string('', 'some-encrypted-data');
            assert(false, 'Should throw error if partition ID is empty');
        } catch (e) {
            // Expected
        }
    } finally {
        await asherah_shutdown_async();
    }
}

async function bad_empty_partition_id_decrypt_async(verbose: boolean, session_cache: boolean, max_stack_alloc_item_size: number): Promise<void> {
    await asherah_setup_static_memory_async(verbose, session_cache, max_stack_alloc_item_size);
    try {
        const { decrypt_string_async } = await import('../dist/asherah');
        await assert_throws_async(async () => {
            await decrypt_string_async('', 'some-encrypted-data');
        }, 'Should throw error if partition ID is empty');
    } finally {
        await asherah_shutdown_async();
    }
}

function get_test_name(prefix: string, verbose: boolean, session_cache: boolean, max_stack_alloc_item_size: number): string {
    return prefix + ' (' + (max_stack_alloc_item_size == 0 ? 'heap' : 'stack ' + max_stack_alloc_item_size) + ') Cache: ' + session_cache + ' Verbose: ' + verbose;
}
