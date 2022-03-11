import { allocate_cbuffer, buffer_to_cbuffer, buffer_to_int64, cbuffer_to_buffer, cbuffer_to_string, int64_to_buffer, json_to_cbuffer, load_platform_library, string_to_cbuffer } from 'cobhan';
import fs from 'fs';

/** KeyMeta contains the ID and Created timestamp for an encryption key. */
export type KeyMeta = {
    /** Identifier of parent key */
    ID: string,
    /** Creation time of parent key (UTC Unit time 64bit) */
    Created: string | number
};

/** EnvelopeKeyRecord represents an encrypted key and is the data structure used
to persist the key in our key table. It also contains the meta data
of the key used to encrypt it. */
export type EnvelopeKeyRecord = {
    /** Encrypted version of the key used to encrypt Data */
    EncryptedKey: Buffer,
    /** Creation time of Key (UTC Unix time 64bit) */
    Created: string | number,
    /** Metadata for the parent key used to envelope encrypt the EncryptedKey */
    ParentKeyMeta: KeyMeta
}

/** DataRowRecord contains the encrypted key and provided data, as well as the information
required to decrypt the key encryption key. This struct should be stored in your
data persistence as it's required to decrypt data. */
export type DataRowRecord = {
    /** Encrypted version of the Data */
    Data: Buffer,
    /** Envelope encryption key record used to encrypt the Data */
    Key: EnvelopeKeyRecord
}

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
    RegionMap: string | null,
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
    'Encrypt': ['int32', ['pointer', 'pointer', 'pointer', 'pointer', 'pointer', 'pointer', 'pointer']],
    'Decrypt': ['int32', ['pointer', 'pointer', 'pointer', 'int64', 'pointer', 'int64', 'pointer']],
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

export function decrypt(partitionId: string, dataRowRecord: DataRowRecord): Buffer {
    const partitionIdBuffer = string_to_cbuffer(partitionId);
    const encryptedDataBuffer = buffer_to_cbuffer(dataRowRecord.Data);
    const encryptedKeyBuffer = buffer_to_cbuffer(dataRowRecord.Key.EncryptedKey);
    const created = dataRowRecord.Key.Created;
    const parentKeyIdBuffer = string_to_cbuffer(dataRowRecord.Key.ParentKeyMeta.ID);
    const parentKeyCreated = dataRowRecord.Key.ParentKeyMeta.Created;

    const outputDataBuffer = allocate_cbuffer(encryptedDataBuffer.length + 256);

    const result = libasherah.Decrypt(partitionIdBuffer, encryptedDataBuffer, encryptedKeyBuffer, created, parentKeyIdBuffer, parentKeyCreated, outputDataBuffer);
    if (result < 0) {
        throw new Error('decrypt failed: ' + result);
    }

    return cbuffer_to_buffer(outputDataBuffer);
}

export function encrypt(partitionId: string, data: Buffer): DataRowRecord {
    const partitionIdBuffer = string_to_cbuffer(partitionId);
    const dataBuffer = buffer_to_cbuffer(data);
    const outputEncryptedDataBuffer = allocate_cbuffer(data.length + 256);
    const outputEncryptedKeyBuffer = allocate_cbuffer(256);
    const outputCreatedBuffer = int64_to_buffer(0);
    const outputParentKeyIdBuffer = allocate_cbuffer(256);
    const outputParentKeyCreatedBuffer = int64_to_buffer(0);

    const result = libasherah.Encrypt(partitionIdBuffer, dataBuffer, outputEncryptedDataBuffer, outputEncryptedKeyBuffer,
        outputCreatedBuffer, outputParentKeyIdBuffer, outputParentKeyCreatedBuffer);

    if (result < 0) {
        throw new Error('encrypt failed: ' + result);
    }
    const parentKeyId = cbuffer_to_string(outputParentKeyIdBuffer);
    const dataRowRecord: DataRowRecord = {
        Data: cbuffer_to_buffer(outputEncryptedDataBuffer),
        Key: {
            EncryptedKey: cbuffer_to_buffer(outputEncryptedKeyBuffer),
            Created: buffer_to_int64(outputCreatedBuffer),
            ParentKeyMeta: {
                ID: parentKeyId,
                Created: buffer_to_int64(outputParentKeyCreatedBuffer)
            }
        }
    };

    return dataRowRecord;
}

export function decrypt_from_json_string(partitionId: string, dataRowRecord: string): Buffer {
  const partitionIdBuffer = string_to_cbuffer(partitionId);
  const jsonBuffer = string_to_cbuffer(dataRowRecord);
  const outputDataBuffer = allocate_cbuffer(jsonBuffer.byteLength);

  const result = libasherah.DecryptFromJson(partitionIdBuffer, jsonBuffer, outputDataBuffer);
  if (result < 0) {
      throw new Error('decrypt failed: ' + result);
  }

  return cbuffer_to_buffer(outputDataBuffer);
}

export function encrypt_to_json_string(partitionId: string, data: Buffer): string {
  const partitionIdBuffer = string_to_cbuffer(partitionId);
  const dataBuffer = buffer_to_cbuffer(data);
  const outputJsonBuffer = allocate_cbuffer(data.byteLength + 256);

  const result = libasherah.EncryptToJson(partitionIdBuffer, dataBuffer, outputJsonBuffer)

  if (result < 0) {
      throw new Error('encrypt failed: ' + result);
  }

  return cbuffer_to_string(outputJsonBuffer);
}
