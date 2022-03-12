import { assert } from 'chai';
import { AsherahConfig, decrypt, encrypt, encrypt_string, decrypt_string, setup, shutdown } from '../src/asherah'
import crypto from 'crypto';
import Benchmark = require('benchmark');

describe('Asherah', function () {
  it('Round Trip Buffers', function () {
    const config: AsherahConfig = {
      KMS: 'static',
      Metastore: 'memory',
      ServiceName: 'TestService',
      ProductID: 'TestProduct',
      Verbose: true,
      EnableSessionCaching: true,
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
      EnableRegionSuffix: null
    };

    setup(config)

    const input = 'mysecretdata'

    const data = Buffer.from(input, 'utf8');

    const encrypted = encrypt('partition', data);

    //Ensure that the secret data isn't anywhere in the output of encrypt
    assert(encrypted.indexOf(input) == -1);

    const decrypted = decrypt('partition', encrypted);

    const output = decrypted.toString('utf8');

    shutdown()

    assert(input == output)
  });
  it('Test RegionMap', function() {
    const config: AsherahConfig = {
      KMS: 'aws',
      Metastore: 'memory',
      ServiceName: 'TestService',
      ProductID: 'TestProduct',
      Verbose: true,
      EnableSessionCaching: true,
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
  it('Round Trip Strings', function () {
    const config: AsherahConfig = {
      KMS: 'static',
      Metastore: 'memory',
      ServiceName: 'TestService',
      ProductID: 'TestProduct',
      Verbose: true,
      EnableSessionCaching: true,
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
      EnableRegionSuffix: null
    };

    setup(config)

    const input = 'mysecretdata'

    const encrypted = encrypt_string('partition', input);

    //Ensure that the secret data isn't anywhere in the output of encrypt
    assert(encrypted.indexOf(input) == -1);

    const output = decrypt_string('partition', encrypted);

    shutdown()

    assert(input == output)
  });
  it('Benchmark RoundTrip', function() {
    this.timeout(0);
    setup({
      KMS: 'static',
      Metastore: 'memory',
      ServiceName: 'TestService',
      ProductID: 'TestProduct',
      Verbose: false,
      EnableSessionCaching: true,
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
      EnableRegionSuffix: null
    });

    const suite = new Benchmark.Suite;
    const input = Buffer.from(JSON.stringify({ key1: 'value1b', nested: { secret: crypto.randomBytes(1024).toString('base64') } }, ['nested.secret']))
    suite.add('RoundTrip#String', function() {
        const enc = encrypt('partition', input);
        decrypt('partition', enc);
    })
    .on('complete', function() {
      const fastest = suite.filter('fastest');
      console.log('RoundTrip mean ' + (fastest.map('stats')[0]['mean'] * 1000).toFixed(3) + 'ms');
    })
    .run({ 'async': false });

    shutdown();
  });
  it('Benchmark Encrypt', function() {
    this.timeout(0);
    setup({
      KMS: 'static',
      Metastore: 'memory',
      ServiceName: 'TestService',
      ProductID: 'TestProduct',
      Verbose: false,
      EnableSessionCaching: true,
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
      EnableRegionSuffix: null
    });

    const suite = new Benchmark.Suite;
    const input = Buffer.from(JSON.stringify({ key1: 'value1b', nested: { secret: crypto.randomBytes(1024).toString('base64') } }, ['nested.secret']))
    suite.add('Encrypt#String', function() {
        encrypt('partition', input);
    })
    .on('complete', function() {
      const fastest = suite.filter('fastest');
      console.log('Encrypt mean ' + (fastest.map('stats')[0]['mean'] * 1000).toFixed(3) + 'ms');
    })
    .run({ 'async': false });

    shutdown();
  });
  it('Benchmark Decrypt', function() {
    this.timeout(0);
    setup({
      KMS: 'static',
      Metastore: 'memory',
      ServiceName: 'TestService',
      ProductID: 'TestProduct',
      Verbose: false,
      EnableSessionCaching: true,
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
      EnableRegionSuffix: null
    });

    const suite = new Benchmark.Suite;
    const input = Buffer.from(JSON.stringify({ key1: 'value1b', nested: { secret: crypto.randomBytes(1024).toString('base64') } }, ['nested.secret']))
    const enc = encrypt('partition', input);
    suite.add('Decrypt#String', function() {
        decrypt('partition', enc);
    })
    .on('complete', function() {
      const fastest = suite.filter('fastest');
      console.log('Decrypt mean ' + (fastest.map('stats')[0]['mean'] * 1000).toFixed(3) + 'ms');
    })
    .run({ 'async': false });

    shutdown();
  });
});


