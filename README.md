# asherah-node

Asherah envelope encryption and key rotation library

This is a wrapper of the Asherah Go implementation using the Cobhan FFI library

## Bun Runtime Support

asherah-node now supports [Bun](https://bun.sh) runtime out of the box! No additional setup required:

```javascript
// Works automatically in both Node.js and Bun
const asherah = require('asherah-node');

// Use normally - Bun compatibility is handled automatically
asherah.setup({ /* config */ });
```

The package automatically detects the Bun runtime and initializes Go compatibility as needed. Set `ASHERAH_BUN_VERBOSE=1` to see Bun initialization logs.

*NOTE:* Due to limitations around the type of libraries Go creates and the type of libraries musl libc supports, you MUST use a glibc based Linux distribution with asherah-node, such as Debian, Ubuntu, AlmaLinux, etc.  Alpine Linux with musl libc will not work.  For technical details, see below.

Example code: 

### TypeScript

```typescript
import { AsherahConfig, decrypt, encrypt, setup, shutdown } from 'asherah'

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
    RegionMap: {"us-west-2": "arn:aws:kms:us-west-2:XXXXXXXXX:key/XXXXXXXXXX"},
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

### JavaScript

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
    RegionMap: {"us-west-2": "arn:aws:kms:us-west-2:XXXXXXXXX:key/XXXXXXXXXX"},
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

### Environment Variables and AWS

If you're experiencing issues with AWS credentials, you can forcibly set the environment variables prior to calling setup in such a way as to ensure they're set for the Go runtime:

```javascript

const asherah = require('asherah');
const fs = require('fs');

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
    RegionMap: {"us-west-2": "arn:aws:kms:us-west-2:XXXXXXXXX:key/XXXXXXXXXX"},
    PreferredRegion: null,
    EnableRegionSuffix: null
  };

// Read the AWS environment variables from the JSON file
// DO NOT HARDCODE YOUR AWS CREDENTIALS
const awsEnvPath = './awsEnv.json';
const awsEnvData = fs.readFileSync(awsEnvPath, 'utf8');
const awsEnv = JSON.stringify(awsEnvData);

// Set the environment variables using the setenv function
asherah.setenv(awsEnv);

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

The `awsEnv.json` file would look like this (spelling errors intentional):

```json
{
  "AXS_ACCESS_KEY_XD": "sample_access_key_xd",
  "AXS_SXCRET_ACCXSS_KEY": "sample_sxcret_accxss_kxy",
  "AXS_SXSSION_TXKEN": "sample_sxssion_txken"
}
```

### Go and Alpine / musl libc

The Golang compiler when creating shared libraries (.so) uses a Thread Local Storage model of init-exec.  This model is inheriently incompatible with loading libraries at runtime with dlopen(), unless your libc reserves some space for dlopen()'ed libraries which is something of a hack.  The most common libc, glibc does in fact reserve space for dlopen()'ed libraries that use init-exec model.  The libc provided with Alpine is musl libc, and it does not participate in this hack / workaround of reserving space.  Most compilers generate libraries with a Thread Local Storage model of global-dynamic which does not require this workaround, and the authors of musl libc do not feel that workaround should exist.

## Updating npm packages

To update packages, run `npm run update`. This command uses [npm-check-updates](https://github.com/raineorshine/npm-check-updates) to bring all npm packages to their latest version. This command also runs `npm install` and `npm audit fix` for you.
