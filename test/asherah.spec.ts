import { assert } from 'chai';
import { AsherahConfig, decrypt, encrypt, decrypt_string, setup, shutdown, encrypt_string } from '../src/asherah'
import crypto from 'crypto';
import Benchmark = require('benchmark');


function get_sample_json(): string {
  return JSON.stringify({ key1: 'value1b', nested: { secret: crypto.randomBytes(1024).toString('base64') } }, ['nested.secret']);
}

function setup_asherah_static_memory(verbose: boolean, session_cache: boolean) {
  setup({
    KMS: 'static',
    Metastore: 'memory',
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
  });
}

function round_trip_buffers(inputBuffer: Buffer) {
  try {
    const encrypted = encrypt('partition', inputBuffer);

    //Ensure that the partition name hasn't been corrupted / truncated
    assert(encrypted.indexOf('partition') != -1);

    //Ensure that there are no NULLs in the encrypted JSON
    assert(encrypted.indexOf('\u0000') == -1);

    const decrypted = decrypt('partition', encrypted);

    //Ensure the decrypted data matches the original input
    assert(Buffer.compare(inputBuffer, decrypted) == 0);
  } finally {
    shutdown();
  }
}

function round_trip_strings(input: string) {
  try {
    const encrypted = encrypt_string('partition', input);

    //Ensure that the secret data isn't anywhere in the output of encrypt
    assert(encrypted.indexOf(input) == -1);

    //Ensure that the partition name hasn't been corrupted / truncated
    assert(encrypted.indexOf('partition') != -1);

    //Ensure that there are no NULLs in the encrypted JSON
    assert(encrypted.indexOf('\u0000') == -1);

    const output = decrypt_string('partition', encrypted);

    //Ensure the decrypted data matches the original input
    assert(input == output);
  } finally {
    shutdown();
  }
}

describe('Asherah', function () {
  it('Round Trip Buffers', function () {
    setup_asherah_static_memory(true, true);
    const data = Buffer.from('mysecretdata', 'utf8');
    round_trip_buffers(data);
  });
  it('Round Trip Strings', function () {
    setup_asherah_static_memory(true, true);
    round_trip_strings('mysecretdata');
  });
  it('Round Trip Buffers (No Session Cache)', function () {
    setup_asherah_static_memory(true, false);
    const data = Buffer.from('mysecretdata', 'utf8');
    round_trip_buffers(data);
  });
  it('Round Trip Strings (No Session Cache)', function () {
    setup_asherah_static_memory(true, false);
    round_trip_strings('mysecretdata');
  });
  it('Benchmark RoundTrip Buffers', function() {
    this.timeout(0);
    setup_asherah_static_memory(false, false);
    try {
      const suite = new Benchmark.Suite;
      const input = Buffer.from(get_sample_json())
      suite.add('RoundTrip#Buffer', function() {
          const enc = encrypt('partition', input);
          decrypt('partition', enc);
      })
      .on('complete', function() {
        const fastest = suite.filter('fastest');
        console.log('RoundTrip Buffers mean ' + (fastest.map('stats')[0]['mean'] * 1000).toFixed(3));
      })
      .run({ 'async': false });
    } finally {
      shutdown();
    }
  });
  it('Benchmark Encrypt Buffers', function() {
    this.timeout(0);
    setup_asherah_static_memory(false, false);
    try {
      const suite = new Benchmark.Suite;
      const input = Buffer.from(get_sample_json());
      suite.add('Encrypt#Buffer', function() {
        encrypt('partition', input);
      })
      .on('complete', function() {
        const fastest = suite.filter('fastest');
        console.log('Encrypt Buffers mean ' + (fastest.map('stats')[0]['mean'] * 1000).toFixed(3));
      })
      .run({ 'async': false });
    } finally {
      shutdown();
    }
  });
  it('Benchmark Decrypt Buffers', function() {
    this.timeout(0);
    setup_asherah_static_memory(false, false);
    try {
      const suite = new Benchmark.Suite;
      const input = Buffer.from(get_sample_json());
      const enc = encrypt('partition', input);
      suite.add('Decrypt#Buffer', function() {
          decrypt('partition', enc);
      })
      .on('complete', function() {
        const fastest = suite.filter('fastest');
        console.log('Decrypt Buffers mean ' + (fastest.map('stats')[0]['mean'] * 1000).toFixed(3));
      })
      .run({ 'async': false });
    } finally {
      shutdown();
    }
  });
  it('Benchmark RoundTrip Strings', function() {
    this.timeout(0);
    setup_asherah_static_memory(false, false);
    try {
      const suite = new Benchmark.Suite;
      const input = get_sample_json();
      suite.add('RoundTrip#String', function() {
          const enc = encrypt_string('partition', input);
          decrypt_string('partition', enc);
      })
      .on('complete', function() {
        const fastest = suite.filter('fastest');
        console.log('RoundTrip Strings mean ' + (fastest.map('stats')[0]['mean'] * 1000).toFixed(3));
      })
      .run({ 'async': false });
    } finally {
      shutdown();
    }
  });
  it('Benchmark Encrypt Strings', function() {
    this.timeout(0);
    setup_asherah_static_memory(false, false);
    try {
      const suite = new Benchmark.Suite;
      const input = get_sample_json();
      suite.add('Encrypt#String', function() {
        encrypt_string('partition', input);
      })
      .on('complete', function() {
        const fastest = suite.filter('fastest');
        console.log('Encrypt Strings mean ' + (fastest.map('stats')[0]['mean'] * 1000).toFixed(3));
      })
      .run({ 'async': false });
    } finally {
      shutdown();
    }
  });
  it('Benchmark Decrypt Strings', function() {
    this.timeout(0);
    setup_asherah_static_memory(false, false);
    try {
      const suite = new Benchmark.Suite;
      const input = get_sample_json();
      const enc = encrypt_string('partition', input);
      suite.add('Decrypt#String', function() {
          decrypt_string('partition', enc);
      })
      .on('complete', function() {
        const fastest = suite.filter('fastest');
        console.log('Decrypt Strings mean ' + (fastest.map('stats')[0]['mean'] * 1000).toFixed(3));
      })
      .run({ 'async': false });
    } finally {
      shutdown();
    }
  });
  it('Test RegionMap', function() {
    const config: AsherahConfig = {
      KMS: 'aws',
      Metastore: 'memory',
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
      RegionMap: {"us-west-2": "arn:aws:kms:us-west-2:795066905288:key/3a628d06-9db4-4b1f-9f76-54fc742dc662"},
      PreferredRegion: null,
      EnableRegionSuffix: null
    };
    setup(config)
    shutdown()
  });
});






