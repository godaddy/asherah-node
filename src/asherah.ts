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
    const configStr = JSON.stringify(config);
    napi_asherah.Napi_SetupJson(configStr, config.ProductID.length, config.ServiceName.length, configStr.length);
}

export function shutdown() {
  napi_asherah.Napi_Shutdown();
}

export function decrypt(partitionId: string, dataRowRecord: string): Buffer {
  return napi_asherah.Napi_DecryptFromJsonToBuffer(partitionId, dataRowRecord);
}

export function encrypt(partitionId: string, data: Buffer): string {
  return napi_asherah.Napi_EncryptFromBufferToJson(partitionId, data)
}

export function decrypt_string(partitionId: string, dataRowRecord: string): string {
  return napi_asherah.Napi_DecryptFromJsonToString(partitionId, dataRowRecord);
}

export function encrypt_string(partitionId: string, data: string): string {
  return napi_asherah.Napi_EncryptFromStringToJson(partitionId, data);
}

export function set_max_stack_alloc_item_size(max_item_size: number) {
  return napi_asherah.Napi_SetMaxStackAllocItemSize(max_item_size);
}
