import { allocate_cbuffer, buffer_to_cbuffer, cbuffer_to_buffer, cbuffer_to_string, json_to_cbuffer, load_platform_library, string_to_cbuffer } from 'cobhan';
import fs from 'fs';

export type AsherahConfig = {
    /** The name of this service (Required) */
    ServiceName: string,
    /** The name of the product that owns this service (Required) */
    ProductID: string,
    /** The amount of time a key is considered valid */
    ExpireAfter: number | null,
    /** The amount of time before cached keys are considered stale */
    CheckInterval: number | null,
    /** Determines the type of metastore to use for persisting keys (Required) { "rdbms", "dynamodb", "memory" } */
    Metastore: string,
    /** The database connection string (Required if metastore=rdbms) */
    ConnectionString: string | null,
    /** Required for Aurora sessions using write forwarding { "eventual", "global", "session" } */
    ReplicaReadConsistency: string | null,
    /** An optional endpoint URL (hostname only or fully qualified URI) (only supported by metastore=dynamodb) */
    DynamoDBEndpoint: string | null,
    /** The AWS region for DynamoDB requests (defaults to globally configured region) (only supported by metastore=dynamodb) */
    DynamoDBRegion: string | null,
    /** The table name for DynamoDB (only supported by metastore=dynamodb) */
    DynamoDBTableName: string | null,
    /** Define the maximum number of sessions to cache (Default 1000) */
    SessionCacheMaxSize: number | null
    /** The amount of time a session will remain cached (Default 2h) */
    SessionCacheDuration: number | null
    /** Configures the master key management service (Default kms) { "aws", "static" } */
    KMS: string | null,
    /** A comma separated list of key-value pairs in the form of REGION1=ARN1[,REGION2=ARN2] (required if kms=aws) */
    RegionMap: { [name: string]: string } | null,
    /** The preferred AWS region (required if kms=aws) */
    PreferredRegion: string | null,
    /** Configure the metastore to use regional suffixes (only supported by metastore=dynamodb) */
    EnableRegionSuffix: boolean | null,
    /** Enable shared session caching */
    EnableSessionCaching: boolean | null,
    /** Enable verbose logging output */
    Verbose: boolean | null,
}

const binaries_path = find_binaries()

const libasherah = load_platform_library(binaries_path, 'libasherah', {
    'SetupJson': ['int32', ['pointer']],
    'EncryptToJson': ['int32', ['pointer', 'pointer', 'pointer']],
    'DecryptFromJson': ['int32', ['pointer', 'pointer', 'pointer']],
    'Shutdown': ['void', []]
});

function find_binaries(): string {
    if (fs.existsSync('node_modules/asherah/binaries')) {
        return 'node_modules/asherah/binaries';
    }
    if (fs.existsSync('binaries')) {
        return 'binaries';
    }
    throw new Error("Could not locate Asherah binaries!")
}

export function setup(config: AsherahConfig) {
    const configJsonBuffer = json_to_cbuffer(config);
    const result = libasherah.SetupJson(configJsonBuffer);
    if (result < 0) {
        throw new Error('setupJson failed: ' + result);
    }
}

export function shutdown() {
  libasherah.Shutdown();
}

export function decrypt(partitionId: string, dataRowRecord: string): Buffer {
  const partitionIdBuffer = string_to_cbuffer(partitionId);
  const jsonBuffer = string_to_cbuffer(dataRowRecord);
  const outputDataBuffer = allocate_cbuffer(jsonBuffer.byteLength);

  const result = libasherah.DecryptFromJson(partitionIdBuffer, jsonBuffer, outputDataBuffer);
  if (result < 0) {
      throw new Error('decrypt failed: ' + result);
  }

  return cbuffer_to_buffer(outputDataBuffer);
}

export function encrypt(partitionId: string, data: Buffer): string {
  const json_overhead = 256;
  const partitionIdBuffer = string_to_cbuffer(partitionId);
  const dataBuffer = buffer_to_cbuffer(data);
  const outputJsonBuffer = allocate_cbuffer(data.byteLength + json_overhead);

  const result = libasherah.EncryptToJson(partitionIdBuffer, dataBuffer, outputJsonBuffer)

  if (result < 0) {
      throw new Error('encrypt failed: ' + result);
  }

  return cbuffer_to_string(outputJsonBuffer);
}

export function decrypt_string(partitionId: string, dataRowRecord: string): string {
  return decrypt(partitionId, dataRowRecord).toString('utf8');
}

export function encrypt_string(partitionId: string, data: string): string {
  return encrypt(partitionId, Buffer.from(data, 'utf8'))
}
