// noinspection JSUnusedGlobalSymbols,DuplicatedCode

/// <reference types="node" />
export type AsherahConfig = {
    /** The name of this service (Required) */
    ServiceName: string;
    /** The name of the product that owns this service (Required) */
    ProductID: string;
    /** The amount of time a key is considered valid */
    ExpireAfter: number | null;
    /** The amount of time before cached keys are considered stale */
    CheckInterval: number | null;
    /** Determines the type of metastore to use for persisting keys (Required) { "rdbms", "dynamodb", "memory" } */
    Metastore: string;
    /** The database connection string (Required if metastore=rdbms) */
    ConnectionString: string | null;
    /** Required for Aurora sessions using write forwarding { "eventual", "global", "session" } */
    ReplicaReadConsistency: string | null;
    /** An optional endpoint URL (hostname only or fully qualified URI) (only supported by metastore=dynamodb) */
    DynamoDBEndpoint: string | null;
    /** The AWS region for DynamoDB requests (defaults to globally configured region) (only supported by metastore=dynamodb) */
    DynamoDBRegion: string | null;
    /** The table name for DynamoDB (only supported by metastore=dynamodb) */
    DynamoDBTableName: string | null;
    /** Define the maximum number of sessions to cache (Default 1000) */
    SessionCacheMaxSize: number | null;
    /** The amount of time a session will remain cached (Default 2h) */
    SessionCacheDuration: number | null;
    /** Configures the master key management service (Default kms) { "aws", "static" } */
    KMS: string | null;
    /** Dictionary of REGION: ARN (required if kms=aws) */
    RegionMap: {
        [name: string]: string;
    } | null;
    /** The preferred AWS region (required if kms=aws) */
    PreferredRegion: string | null;
    /** Configure the metastore to use regional suffixes (only supported by metastore=dynamodb) */
    EnableRegionSuffix: boolean | null;
    /** Enable shared session caching */
    EnableSessionCaching: boolean | null;
    /** Enable verbose logging output */
    Verbose: boolean | null;
};

type LogHookCallback = (level: number, message: string) => void;

export declare function setup(config: AsherahConfig): void;
export declare function setup_async(config: AsherahConfig): Promise;
export declare function shutdown(): void;
export declare function shutdown_async(): Promise;
export declare function decrypt(partitionId: string, dataRowRecord: string): Buffer;
export declare function decrypt_async(partitionId: string, dataRowRecord: string): Promise<Buffer>;
export declare function encrypt(partitionId: string, data: Buffer): string;
export declare function encrypt_async(partitionId: string, data: Buffer): Promise<string>;
export declare function decrypt_string(partitionId: string, dataRowRecord: string): string;
export declare function decrypt_string_async(partitionId: string, dataRowRecord: string): Promise<string>;
export declare function encrypt_string(partitionId: string, data: string): string;
export declare function encrypt_string_async(partitionId: string, data: string): Promise<string>;
export declare function set_max_stack_alloc_item_size(max_item_size: number): void;
export declare function set_safety_padding_overhead(safety_padding_overhead: number): void;
export declare function set_log_hook(logHook: LogHookCallback): void;
export declare function get_setup_status(): boolean;
export declare function setenv(environment: string): void;
