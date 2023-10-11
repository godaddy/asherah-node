const asherah = require('../dist/asherah.node')
const crypto = require('crypto');

/* LOCAL DDB
https://docs.aws.amazon.com/amazondynamodb/latest/developerguide/DynamoDBLocal.DownloadingAndRunning.html#DynamoDBLocal.DownloadingAndRunning.title
*/

/* SETUP CREDS (any key & secret will do)
aws configure --profile local
*/

/* CREATE TABLE
aws dynamodb create-table \
    --table-name METASTORE_TABLE_NAME \
    --attribute-definitions \
        AttributeName=Id,AttributeType=S \
        AttributeName=Created,AttributeType=N \
    --key-schema AttributeName=Id,KeyType=HASH AttributeName=Created,KeyType=RANGE \
    --provisioned-throughput ReadCapacityUnits=1,WriteCapacityUnits=1 \
    --table-class STANDARD \
    --endpoint-url http://localhost:8000
*/

/* VERIFY
aws dynamodb list-tables --endpoint-url http://localhost:8000
*/

const TESTS_PER_CYCLE = 10;
const RANDOM_INPUT_MIN_LENGTH = 1;
const RANDOM_INPUT_MAX_LENGTH = 100_000_000;

const CONFIG = {
  KMS: 'static',
  Metastore: 'dynamodb',
  DynamoDBRegion: 'us-west-2' || null,
  DynamoDBTableName: 'METASTORE_TABLE_NAME',
  DynamoDBEndpoint: 'http://localhost:8000',
  ServiceName: 'SERVICE_NAME',
  ProductID: 'PRODUCT_ID',
  EnableSessionCaching: true,
  EnableRegionSuffix: true,
  RegionMap: { "us-west-2": "arn:aws:kms:us-west-2:XXXXXXXXX:key/XXXXXXXXXX" },
  ExpireAfter: null,
  CheckInterval: null,
  ConnectionString: null,
  ReplicaReadConsistency: null,
  SessionCacheMaxSize: null,
  SessionCacheDuration: null,
  PreferredRegion: null,
  Verbose: false,
};

asherah.setup(CONFIG);

function runTest() {
  // create random length input
  const inputLength = Math.floor(Math.random() * (RANDOM_INPUT_MAX_LENGTH - RANDOM_INPUT_MIN_LENGTH + 1)) + RANDOM_INPUT_MIN_LENGTH;

  const input = crypto.randomBytes(inputLength);
  const encrypted = asherah.encrypt('partition', input);
  const decrypted = asherah.decrypt('partition', encrypted);
  if (input.byteLength !== decrypted.byteLength) {
    throw new Error(`Expected ${input.byteLength} bytes, got ${decrypted.byteLength} bytes`);
  }

  return input.byteLength;
}

let totalBytes = 0;
let start = performance.now();
async function benchTest() {
  totalBytes += runTest();
  await new Promise(resolve => setImmediate(resolve));
  benchTest();
}

for (let i = 0; i < TESTS_PER_CYCLE; i++) {
  benchTest();
}

setInterval(() => {
  const elapsed = performance.now() - start;
  const throughputMB = totalBytes / elapsed / 1000;
  console.log(`Throughput: ${throughputMB.toFixed(2)} MB/s`);
  start = performance.now();
  totalBytes = 0;
}, 20_000);
