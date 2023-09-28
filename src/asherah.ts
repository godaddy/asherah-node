import { buffer_to_cbuffer, json_to_cbuffer, string_to_cbuffer } from 'cobhan';
const napi_asherah = require('../build/Release/napiasherah.node');

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
    /** Dictionary of REGION: ARN (required if kms=aws) */
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

export function setup(config: AsherahConfig) {
    const configJsonBuffer = json_to_cbuffer(config);
    napi_asherah.Napi_SetupJson(configJsonBuffer);
}

export function shutdown() {
  napi_asherah.Napi_Shutdown();
}

export function decrypt(partitionId: string, dataRowRecord: string): Buffer {
  const partitionIdBuffer = string_to_cbuffer(partitionId);
  const jsonBuffer = string_to_cbuffer(dataRowRecord);

  return napi_asherah.Napi_DecryptFromJson(partitionIdBuffer, jsonBuffer);
}

export function encrypt(partitionId: string, data: Buffer): string {
  const partitionIdBuffer = string_to_cbuffer(partitionId);
  const dataBuffer = buffer_to_cbuffer(data);

  return napi_asherah.Napi_EncryptToJson(partitionIdBuffer, dataBuffer)
}

export function decrypt_string(partitionId: string, dataRowRecord: string): string {
  return decrypt(partitionId, dataRowRecord).toString('utf8');
}

export function encrypt_string(partitionId: string, data: string): string {
  return encrypt(partitionId, Buffer.from(data, 'utf8'))
}
