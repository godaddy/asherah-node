import { allocate_cbuffer, buffer_to_cbuffer, buffer_to_int64, cbuffer_to_buffer, cbuffer_to_string, int64_to_buffer, load_platform_library, string_to_cbuffer } from 'cobhan';

// KeyMeta contains the ID and Created timestamp for an encryption key.

export type KeyMeta = {
    ID: string,
    Created: string | number
};

// EnvelopeKeyRecord represents an encrypted key and is the data structure used
// to persist the key in our key table. It also contains the meta data
// of the key used to encrypt it.

export type EnvelopeKeyRecord = {
    EncryptedKey: Buffer,
    Created: string | number,
    ParentKeyMeta: KeyMeta
}

// DataRowRecord contains the encrypted key and provided data, as well as the information
// required to decrypt the key encryption key. This struct should be stored in your
// data persistence as it's required to decrypt data.

export type DataRowRecord = {
    Data: Buffer,
    Key: EnvelopeKeyRecord
}

const libasherah = load_platform_library('node_modules/asherah/binaries', 'libasherah', {
    'Encrypt': ['int32', ['pointer', 'pointer', 'pointer', 'pointer', 'pointer', 'pointer', 'pointer']],
    'Decrypt': ['int32', ['pointer', 'pointer', 'pointer', 'int64', 'pointer', 'int64', 'pointer']],
    'Setup': ['int32', ['pointer', 'pointer', 'pointer', 'pointer', 'pointer', 'pointer', 'int32', 'pointer', 'pointer', 'pointer', 'pointer', 'int32', 'int32', 'int32']],
});

export type AsherahConfig = {
    kmsType: string,
    metastore: string,
    serviceName: string,
    productId: string,
    rdbmsConnectionString: string | null,
    dynamoDbEndpoint: string | null,
    dynamoDbRegion: string | null,
    dynamoDbTableName: string | null,
    enableRegionSuffix: boolean,
    preferredRegion: string | null,
    regionMap: string | null,
    verbose: boolean,
    sessionCache: boolean,
    debugOutput: boolean
}

export function setup(config: AsherahConfig) {
    const kmsTypeBuffer = string_to_cbuffer(config.kmsType)
    const metastoreBuffer = string_to_cbuffer(config.metastore)
    const rdbmsConnectionStringBuffer = string_to_cbuffer(config.rdbmsConnectionString)
    const dynamoDbEndpointBuffer = string_to_cbuffer(config.dynamoDbEndpoint)
    const dynamoDbRegionBuffer = string_to_cbuffer(config.dynamoDbRegion)
    const dynamoDbTableNameBuffer = string_to_cbuffer(config.dynamoDbTableName)
    const enableRegionSuffixInt = config.enableRegionSuffix ? 1 : 0
    const serviceNameBuffer = string_to_cbuffer(config.serviceName)
    const productIdBuffer = string_to_cbuffer(config.productId)
    const preferredRegionBuffer = string_to_cbuffer(config.preferredRegion)
    const regionMapBuffer = string_to_cbuffer(config.regionMap)
    const verboseInt = config.verbose ? 1 : 0
    const sessionCacheInt = config.sessionCache ? 1 : 0
    const debugOutputInt = config.debugOutput ? 1 : 0

    const result = libasherah.Setup(kmsTypeBuffer, metastoreBuffer, rdbmsConnectionStringBuffer, dynamoDbEndpointBuffer, dynamoDbRegionBuffer, dynamoDbTableNameBuffer,
        enableRegionSuffixInt, serviceNameBuffer, productIdBuffer, preferredRegionBuffer, regionMapBuffer, verboseInt, sessionCacheInt, debugOutputInt);

    if (result < 0) {
        throw new Error('setup failed: ' + result);
    }
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
