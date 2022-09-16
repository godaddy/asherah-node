const asherah = require('asherah')

const config = {
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