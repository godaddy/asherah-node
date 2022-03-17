Asherah envelope encryption and key rotation library

This is a wrapper of the Asherah Go implementation using the Cobhan FFI library

Example code: 


```typescript
import { AsherahConfig, decrypt, encrypt, setup, shutdown } from 'asherah'

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

console.log("Input: " + input)

const data = Buffer.from(input, 'utf8');

const encrypted = encrypt('partition', data);

const decrypted = decrypt('partition', encrypted);

const output = decrypted.toString('utf8');

console.log("Output: " + output)

shutdown()
```

```javascript

const asherah = require('asherah')

const config = {
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
    RegionMap: {"us-west-2": "arn:aws:kms:us-west-2:669979021785:key/a09cf820-ca91-4199-a4a8-ab0c35efc8d4"},
    PreferredRegion: null,
    EnableRegionSuffix: null
  };

asherah.setup(config)

const input = 'mysecretdata'

console.log("Input: " + input)

const data = Buffer.from(input, 'utf8');

const encrypted = asherah.encrypt('partition', data);

const decrypted = asherah.decrypt('partition', encrypted);

const output = decrypted.toString('utf8');

console.log("Output: " + output)

asherah.shutdown()
```
