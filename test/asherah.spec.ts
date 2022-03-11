import { assert } from 'chai';
import { AsherahConfig, decrypt, encrypt, setup, shutdown } from '../src/asherah'
import crypto from 'crypto';

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
  it('Speed Test', function () {
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

    console.time('encrypt');
    const enc1 = encrypt('partition', Buffer.from(JSON.stringify({ key1: 'value1a', secret1: 'secret1a', nested: { secret2: crypto.randomBytes(256).toString('base64') } }, ['secret1', 'nested.secret2'])));
    console.timeEnd('encrypt');
    console.time('decrypt');
    decrypt('partition', enc1);
    console.timeEnd('decrypt');

    console.time('encrypt');
    const enc2 = encrypt('partition', Buffer.from(JSON.stringify({ key1: 'value1b', secret1: 'secret1b', nested: { secret2: crypto.randomBytes(256).toString('base64') } }, ['secret1', 'nested.secret2'])));
    console.timeEnd('encrypt');
    console.time('decrypt');
    decrypt('partition', enc2);
    console.timeEnd('decrypt');

    let enc;
    console.time('encryptAndDecrypt x 1000 x 1KB');
    for (let i = 0; i < 1000; i++) {
      enc = encrypt('partition', Buffer.from(JSON.stringify({ key1: 'value1b', nested: { secret: crypto.randomBytes(1024).toString('base64') } }, ['nested.secret'])));
      decrypt('partition', enc);
    }
    console.timeEnd('encryptAndDecrypt x 1000 x 1KB');
  });
});


