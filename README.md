Asherah envelope encryption and key rotation library

This is a wrapper of the Asherah Go implementation using the Cobhan FFI library

Example code: 


```typescript
import { AsherahConfig, decrypt, encrypt, setup, shutdown } from '../src/asherah'

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
```
