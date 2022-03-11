import { assert } from 'chai';
import { AsherahConfig, decrypt, decrypt_from_json_string, encrypt, encrypt_to_json_string, setup, shutdown } from '../src/asherah'
import crypto from 'crypto';
import Benchmark = require('benchmark');

describe('Asherah', function () {
  it('Round Trip', function () {
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

    const decrypted = decrypt('partition', encrypted);

    const output = decrypted.toString('utf8');

    shutdown()

    assert(input == output)
  });
  it('Benchmarks', function() {
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

    // add tests
    suite.add('RoundTrip#String', function() {
        const enc = encrypt_to_json_string('partition', input);
        decrypt_from_json_string('partition', enc);
    })
    .add('RoundTrip#Object', function() {
      const enc = encrypt('partition', input);
      decrypt('partition', enc);
    })
    .on('complete', function() {
      const fastest = suite.filter('fastest');
      console.log('Fastest is ' + fastest.map('name') + ' with mean ' + (fastest.map('stats')[0]['mean'] * 1000).toFixed(3) + 'ms');
      const slowest = suite.filter('slowest');
      console.log('Slowest is ' + slowest.map('name') + ' with mean ' + (slowest.map('stats')[0]['mean'] * 1000).toFixed(3) + 'ms');
    })
    // run async
    .run({ 'async': false });

    shutdown();
  });
});


