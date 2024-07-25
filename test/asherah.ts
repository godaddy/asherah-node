import {
    AsherahConfig,
    decrypt,
    decrypt_async,
    decrypt_string,
    decrypt_string_async,
    encrypt,
    encrypt_async,
    encrypt_string,
    encrypt_string_async,
    setup,
    setup_async,
    shutdown,
    shutdown_async,
    get_setup_status,
    set_max_stack_alloc_item_size,
    set_log_hook,
    setenv
} from '../dist/asherah'

import { assert } from 'chai';
import winston = require('winston');
import { posix_log_levels } from './helpers';

// Re-export this so callers don't have to import Asherah directly
export type { AsherahConfig }

function get_static_memory_config(verbose: boolean, session_cache: boolean): AsherahConfig {
    return {
        KMS: 'test-debug-static',
        Metastore: 'test-debug-memory',
        ServiceName: 'TestService',
        ProductID: 'TestProduct',
        Verbose: verbose,
        EnableSessionCaching: session_cache,
        ExpireAfter: null,
        CheckInterval: null,
        ConnectionString: null,
        ReplicaReadConsistency: null,
        DynamoDBEndpoint: null,
        DynamoDBRegion: null,
        DynamoDBTableName: null,
        SessionCacheMaxSize: 10000,
        SessionCacheDuration: null,
        RegionMap: null,
        PreferredRegion: null,
        EnableRegionSuffix: null
    };
}

function configure_winston_logging(level = 'error') {
    winston.configure({
        transports: [
            new winston.transports.Console({
                level: level,
                format: winston.format.combine(
                    winston.format.colorize(),
                    winston.format.simple()
                )
            })
        ]
    });

    set_log_hook((level: number, message: string): void => {
        if (level <= posix_log_levels.error) {
            winston.error(message);
            console.error(message);
        } else if (level == posix_log_levels.debug) {
            winston.debug(message);
        } else {
            winston.info(message);
        }
    });
}

export function asherah_setup(config: AsherahConfig) {
    setup(config);
}

export function asherah_setup_static_memory(verbose = false, session_cache = true, max_stack_alloc_item_size = -1): void {
    configure_winston_logging();
    asherah_set_env();
    setup(get_static_memory_config(verbose, session_cache));
    if (max_stack_alloc_item_size >= 0) {
        set_max_stack_alloc_item_size(max_stack_alloc_item_size);
    }
}

export async function asherah_setup_static_memory_async(verbose = false, session_cache = true, max_stack_alloc_item_size = -1): Promise<void> {
    configure_winston_logging();
    asherah_set_env();
    await setup_async(get_static_memory_config(verbose, session_cache));
    if (max_stack_alloc_item_size >= 0) {
        set_max_stack_alloc_item_size(max_stack_alloc_item_size);
    }
}

export function asherah_set_env() {
    const myString = '{"VAR": "VAL"}';
    setenv(myString);
}

export function asherah_shutdown(): void {
    shutdown();
}

export async function asherah_shutdown_async(): Promise<void> {
    await shutdown_async();
}

export function test_round_trip_buffers(inputBuffer: Buffer): void {
    assert_asherah_setup();

    const encrypted = encrypt('partition', inputBuffer);

    const inputString = inputBuffer.toString('utf8');

    assert_encrypt_string('partition', inputString, encrypted);

    const decrypted = decrypt('partition', encrypted);

    assert_buffers_equal(inputBuffer, decrypted, 'Decrypted data should match original input');
}

export async function test_round_trip_buffers_async(inputBuffer: Buffer): Promise<void> {
    assert_asherah_setup();

    const encrypted = await encrypt_async('partition', inputBuffer);

    const inputString = inputBuffer.toString('utf8');

    assert_encrypt_string('partition', inputString, encrypted);

    const decrypted = await decrypt_async('partition', encrypted);

    assert_buffers_equal(inputBuffer, decrypted, 'Decrypted data should match original input');
}

export function test_round_trip_strings(input: string): void {
    assert_asherah_setup();

    const encrypted = encrypt_string('partition', input);

    assert_encrypt_string('partition', input, encrypted);

    const output = decrypt_string('partition', encrypted);

    assert_output_string(input, output);
}

export async function test_round_trip_strings_async(input: string): Promise<void> {
    assert_asherah_setup();

    const encrypted = await encrypt_string_async('partition', input);

    assert_encrypt_string('partition', input, encrypted);

    const output = await decrypt_string_async('partition', encrypted);

    assert_output_string(input, output);
}

export function assert_throws(func: () => void, message: string) {
    try {
        func();
        assert(false, message);
    } catch (e) {
        // Expected
    }
}

export async function assert_throws_async(func: () => Promise<void>, message: string) {
    try {
        await func();
        assert(false, message);
    } catch (e) {
        // Expected
    }
}

export function assert_asherah_setup() {
    assert(get_setup_status(), "Uninitialized Asherah!");
}

export function assert_asherah_shutdown() {
    assert(!get_setup_status(), "Asherah still initialized!");
}

function assert_buffers_equal(a: Buffer, b: Buffer, message?: string) {
    if (Buffer.compare(a, b) != 0) {
        console.error("Buffers are not equal");
        console.error("a size: [" + a.length + "]");
        console.error("b size: [" + b.length + "]");
        console.error("a: [" + a.toString('utf8') + "]");
        console.error("b: [" + b.toString('utf8') + "]");
        assert(false, message ?? "Buffers are not equal");
    }
}

function assert_encrypt_string(partition: string, input: string, encrypted: string) {
    //Ensure that the secret data isn't anywhere in the output of encrypt
    assert(encrypted.indexOf(input) == -1, "Encrypted data should not contain secret input data");

    //Ensure that the partition name hasn't been corrupted / truncated
    assert(encrypted.indexOf(partition) != -1, "Encrypted data should contain partition name");

    //Ensure that there are no NULLs in the encrypted JSON
    assert(encrypted.indexOf('\u0000') == -1, "Encrypted data should not contain NULLs");
}

function assert_output_string(input: string, output: string) {
    //Ensure the decrypted data matches the original input
    assert(input == output, "Decrypted data should match original input");
}

export async function loop_decrypt_string_async(input: string, partition = 'partition', cycles = 1): Promise<void> {
    for (let i = 0; i < cycles; i++) {
        await decrypt_string_async(partition, input);
    }
}

export async function loop_encrypt_string_async(input: string, partition = 'partition', cycles = 1): Promise<void> {
    for (let i = 0; i < cycles; i++) {
        await encrypt_string_async(partition, input);
    }
}

export async function loop_roundtrip_string_async(input: string, partition = 'partition', cycles = 1): Promise<void> {
    for (let i = 0; i < cycles; i++) {
        const enc = await encrypt_string_async(partition, input);
        await decrypt_string_async(partition, enc);
    }
}
export async function loop_decrypt_async(input: string, partition = 'partition', cycles = 1): Promise<void> {
    for (let i = 0; i < cycles; i++) {
        await decrypt_async(partition, input);
    }
}

export async function loop_encrypt_async(input: Buffer, partition = 'partition', cycles = 1): Promise<void> {
    for (let i = 0; i < cycles; i++) {
        await encrypt_async(partition, input);
    }
}

export async function loop_roundtrip_buffer_async(input: Buffer, partition = 'partition', cycles = 1): Promise<void> {
    for (let i = 0; i < cycles; i++) {
        const enc = await encrypt_async(partition, input);
        await decrypt_async(partition, enc);
    }
}
