import { assert } from 'chai';
import { AsherahConfig, decrypt, encrypt, decrypt_string, setup, shutdown, encrypt_string, set_max_stack_alloc_item_size, set_safety_padding_overhead } from '../dist/asherah'
import crypto from 'crypto';
import Benchmark = require('benchmark');

const benchmark = true;

function get_sample_json(): string {
  return JSON.stringify({ key1: 'value1b', nested: { secret: crypto.randomBytes(1024).toString('base64') } }, ['nested.secret']);
}

// Fails at 1623 or larger
function get_big_string(): string {
  return 'x'.repeat(16384);
}

function setup_asherah_static_memory(verbose: boolean, session_cache: boolean) {
  setup({
    KMS: 'static',
    Metastore: 'memory',
    ServiceName: 'TestService',
    ProductID: 'TestProduct',
    Verbose: verbose,
    EnableSessionCaching: session_cache,
    ExpireAfter: null,
    CheckInterval: null,
    ConnectionString: null,
    ReplicaReadConsistency: null,
    DynamoDBEndpoint: null,
    DynamoDBRegion: null,
    DynamoDBTableName: null,
    SessionCacheMaxSize: 10000,
    SessionCacheDuration: null,
    RegionMap: null,
    PreferredRegion: null,
    EnableRegionSuffix: null
  });
}

function round_trip_buffers(inputBuffer: Buffer) {
  try {
    console.error("encrypt buffer");
    const encrypted = encrypt('partition', inputBuffer);

    const inputString = inputBuffer.toString('utf8');
    assert(encrypted.indexOf(inputString) == -1, "encrypted data should not contain secret input data");

    assert(encrypted.indexOf('partition') != -1, "encrypted data should contain partition name");

    assert(encrypted.indexOf('\u0000') == -1, "encrypted data should not contain NULLs");

    const decrypted = decrypt('partition', encrypted);

    assert(Buffer.compare(inputBuffer, decrypted) == 0, "decrypted data should match original input");
  } finally {
    shutdown();
  }
}

function round_trip_strings(input: string) {
  try {
    const encrypted = encrypt_string('partition', input);

    //Ensure that the secret data isn't anywhere in the output of encrypt
    assert(encrypted.indexOf(input) == -1, "encrypted data should not contain secret input data");

    //Ensure that the partition name hasn't been corrupted / truncated
    assert(encrypted.indexOf('partition') != -1, "encrypted data should contain partition name");

    //Ensure that there are no NULLs in the encrypted JSON
    assert(encrypted.indexOf('\u0000') == -1, "encrypted data should not contain NULLs");

    const output = decrypt_string('partition', encrypted);

    //Ensure the decrypted data matches the original input
    assert(input == output, "decrypted data should match original input");
  } finally {
    shutdown();
  }
}

describe('Asherah', function () {
  it('Round Trip Buffers (heap)', function () {
    set_max_stack_alloc_item_size(0);
    setup_asherah_static_memory(true, true);
    const data = Buffer.from('mysecretdata', 'utf8');
    round_trip_buffers(data);
  });
  it('Round Trip Strings (heap)', function () {
    set_max_stack_alloc_item_size(0);
    setup_asherah_static_memory(true, true);
    round_trip_strings('mysecretdata');
  });
  it('Round Trip Buffers (heap) (No Session Cache)', function () {
    set_max_stack_alloc_item_size(0);
    setup_asherah_static_memory(true, false);
    const data = Buffer.from('mysecretdata', 'utf8');
    round_trip_buffers(data);
  });
  it('Round Trip Strings (heap) (No Session Cache)', function () {
    set_max_stack_alloc_item_size(0);
    setup_asherah_static_memory(true, false);
    round_trip_strings('mysecretdata');
  });

  it('Round Trip Buffers (stack)', function () {
    set_max_stack_alloc_item_size(2048);
    setup_asherah_static_memory(true, true);
    const data = Buffer.from('mysecretdata', 'utf8');
    round_trip_buffers(data);
  });
  it('Round Trip Strings (stack)', function () {
    set_max_stack_alloc_item_size(2048);
    setup_asherah_static_memory(true, true);
    round_trip_strings('mysecretdata');
  });
  it('Round Trip Buffers (stack) (No Session Cache)', function () {
    set_max_stack_alloc_item_size(2048);
    setup_asherah_static_memory(true, false);
    const data = Buffer.from('mysecretdata', 'utf8');
    round_trip_buffers(data);
  });
  it('Round Trip Strings (stack) (No Session Cache)', function () {
    set_max_stack_alloc_item_size(2048);
    setup_asherah_static_memory(true, false);
    round_trip_strings('mysecretdata');
  });

  it('Safety Padding Buffers (heap)', function () {
    set_safety_padding_overhead(512);
    set_max_stack_alloc_item_size(0);
    setup_asherah_static_memory(true, true);
    const data = Buffer.from('mysecretdata', 'utf8');
    round_trip_buffers(data);
  });

  it('Safety Padding Strings (heap)', function () {
    set_safety_padding_overhead(512);
    set_max_stack_alloc_item_size(0);
    setup_asherah_static_memory(true, true);
    round_trip_strings('mysecretdata');
  });

  it('Safety Padding Buffers (stack)', function () {
    set_safety_padding_overhead(512);
    set_max_stack_alloc_item_size(2048);
    setup_asherah_static_memory(true, true);
    const data = Buffer.from('mysecretdata', 'utf8');
    round_trip_buffers(data);
  });

  it('Safety Padding Strings (stack)', function () {
    set_safety_padding_overhead(512);
    set_max_stack_alloc_item_size(2048);
    setup_asherah_static_memory(true, true);
    round_trip_strings('mysecretdata');
  });

  it('Round Trip Buffers (heap) (big)', function () {
    set_max_stack_alloc_item_size(0);
    setup_asherah_static_memory(true, true);
    const data = Buffer.from(get_big_string(), 'utf8');
    round_trip_buffers(data);
  });
  it('Round Trip Strings (heap) (big)', function () {
    set_max_stack_alloc_item_size(0);
    setup_asherah_static_memory(true, true);
    round_trip_strings(get_big_string());
  });
  it('Round Trip Buffers (heap) (big) (No Session Cache)', function () {
    set_max_stack_alloc_item_size(0);
    setup_asherah_static_memory(true, false);
    const data = Buffer.from(get_big_string(), 'utf8');
    round_trip_buffers(data);
  });
  it('Round Trip Strings (heap) (big) (No Session Cache)', function () {
    set_max_stack_alloc_item_size(0);
    setup_asherah_static_memory(true, false);
    round_trip_strings(get_big_string());
  });

  it('Round Trip Buffers (stack) (big)', function () {
    set_max_stack_alloc_item_size(65536);
    setup_asherah_static_memory(true, true);
    const data = Buffer.from(get_big_string(), 'utf8');
    round_trip_buffers(data);
  });
  it('Round Trip Strings (stack) (big)', function () {
    set_max_stack_alloc_item_size(65536);
    setup_asherah_static_memory(true, true);
    round_trip_strings(get_big_string());
  });
  it('Round Trip Buffers (stack) (big) (No Session Cache)', function () {
    set_max_stack_alloc_item_size(65536);
    setup_asherah_static_memory(true, false);
    const data = Buffer.from(get_big_string(), 'utf8');
    round_trip_buffers(data);
  });
  it('Round Trip Strings (stack) (big) (No Session Cache)', function () {
    set_max_stack_alloc_item_size(65536);
    setup_asherah_static_memory(true, false);
    round_trip_strings(get_big_string());
  });

  it('Safety Padding Buffers (heap) (big)', function () {
    set_safety_padding_overhead(4096);
    set_max_stack_alloc_item_size(0);
    setup_asherah_static_memory(true, true);
    const data = Buffer.from(get_big_string(), 'utf8');
    round_trip_buffers(data);
  });

  it('Safety Padding Strings (heap) (big)', function () {
    set_safety_padding_overhead(4096);
    set_max_stack_alloc_item_size(0);
    setup_asherah_static_memory(true, true);
    round_trip_strings(get_big_string());
  });

  it('Safety Padding Buffers (stack) (big)', function () {
    set_safety_padding_overhead(4096);
    set_max_stack_alloc_item_size(65536);
    setup_asherah_static_memory(true, true);
    const data = Buffer.from(get_big_string(), 'utf8');
    round_trip_buffers(data);
  });

  it('Safety Padding Strings (stack) (big)', function () {
    set_safety_padding_overhead(4096);
    set_max_stack_alloc_item_size(65536);
    setup_asherah_static_memory(true, true);
    round_trip_strings(get_big_string());
  });

  it('Test RegionMap', function () {
    const config: AsherahConfig = {
      KMS: 'aws',
      Metastore: 'memory',
      ServiceName: 'TestService',
      ProductID: 'TestProduct',
      Verbose: true,
      EnableSessionCaching: false,
      ExpireAfter: null,
      CheckInterval: null,
      ConnectionString: null,
      ReplicaReadConsistency: null,
      DynamoDBEndpoint: null,
      DynamoDBRegion: null,
      DynamoDBTableName: null,
      SessionCacheMaxSize: null,
      SessionCacheDuration: null,
      RegionMap: { "us-west-2": "arn:aws:kms:us-west-2:795066905288:key/3a628d06-9db4-4b1f-9f76-54fc742dc662" },
      PreferredRegion: null,
      EnableRegionSuffix: null
    };
    setup(config)
    shutdown()
  });
});
if(benchmark) {
it('Benchmark RoundTrip Buffers (heap)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(0);
  setup_asherah_static_memory(false, true);
  benchmark_roundtrip_buffer(this.test?.title ?? 'unknown test', get_sample_json());
});
it('Benchmark Encrypt Buffers (heap)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(0);
  setup_asherah_static_memory(false, true);
  benchmark_encrypt_buffer(this.test?.title ?? 'unknown test', get_sample_json());
});
it('Benchmark Decrypt Buffers (heap)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(0);
  setup_asherah_static_memory(false, true);
  benchmark_decrypt_buffer(this.test?.title ?? 'unknown test', get_sample_json());
});
it('Benchmark RoundTrip Strings (heap)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(0);
  setup_asherah_static_memory(false, true);
  benchmark_roundtrip_string(this.test?.title ?? 'unknown test', get_sample_json());
});
it('Benchmark Encrypt Strings (heap)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(0);
  setup_asherah_static_memory(false, true);
  benchmark_encrypt_string(this.test?.title ?? 'unknown test', get_sample_json());
});
it('Benchmark Decrypt Strings (heap)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(0);
  setup_asherah_static_memory(false, true);
  benchmark_decrypt_string(this.test?.title ?? 'unknown test', get_sample_json());
});

it('Benchmark RoundTrip Buffers (stack)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(2048);
  setup_asherah_static_memory(false, true);
  benchmark_roundtrip_buffer(this.test?.title ?? 'unknown test', get_sample_json());
});
it('Benchmark Encrypt Buffers (stack)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(2048);
  setup_asherah_static_memory(false, true);
  benchmark_encrypt_buffer(this.test?.title ?? 'unknown test', get_sample_json());
});
it('Benchmark Decrypt Buffers (stack)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(2048);
  setup_asherah_static_memory(false, true);
  benchmark_decrypt_buffer(this.test?.title ?? 'unknown test', get_sample_json());
});
it('Benchmark RoundTrip Strings (stack)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(2048);
  setup_asherah_static_memory(false, true);
  benchmark_roundtrip_string(this.test?.title ?? 'unknown test', get_sample_json());
});
it('Benchmark Encrypt Strings (stack)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(2048);
  setup_asherah_static_memory(false, true);
  benchmark_encrypt_string(this.test?.title ?? 'unknown test', get_sample_json());
});
it('Benchmark Decrypt Strings (stack)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(2048);
  setup_asherah_static_memory(false, true);
  benchmark_decrypt_string(this.test?.title ?? 'unknown test', get_sample_json());
});

it('Benchmark RoundTrip Buffers (heap) (big)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(0);
  setup_asherah_static_memory(false, true);
  benchmark_roundtrip_buffer(this.test?.title ?? 'unknown test', get_big_string());
});
it('Benchmark Encrypt Buffers (heap) (big)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(0);
  setup_asherah_static_memory(false, true);
  benchmark_encrypt_buffer(this.test?.title ?? 'unknown test', get_big_string());
});
it('Benchmark Decrypt Buffers (heap) (big)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(0);
  setup_asherah_static_memory(false, true);
  benchmark_decrypt_buffer(this.test?.title ?? 'unknown test', get_big_string());
});
it('Benchmark RoundTrip Strings (heap) (big)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(0);
  setup_asherah_static_memory(false, true);
  benchmark_roundtrip_string(this.test?.title ?? 'unknown test', get_big_string());
});
it('Benchmark Encrypt Strings (heap) (big)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(0);
  setup_asherah_static_memory(false, true);
  benchmark_encrypt_string(this.test?.title ?? 'unknown test', get_big_string());
});
it('Benchmark Decrypt Strings (heap) (big)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(0);
  setup_asherah_static_memory(false, true);
  benchmark_decrypt_string(this.test?.title ?? 'unknown test', get_big_string());
});

it('Benchmark RoundTrip Buffers (stack) (big)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(65536);
  setup_asherah_static_memory(false, true);
  benchmark_roundtrip_buffer(this.test?.title ?? 'unknown test', get_big_string());
});
it('Benchmark Encrypt Buffers (stack) (big)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(65536);
  setup_asherah_static_memory(false, true);
  benchmark_encrypt_buffer(this.test?.title ?? 'unknown test', get_big_string());
});
it('Benchmark Decrypt Buffers (stack) (big)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(65536);
  setup_asherah_static_memory(false, true);
  benchmark_decrypt_buffer(this.test?.title ?? 'unknown test', get_big_string());
});
it('Benchmark RoundTrip Strings (stack) (big)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(65536);
  setup_asherah_static_memory(false, true);
  benchmark_roundtrip_string(this.test?.title ?? 'unknown test', get_big_string());
});
it('Benchmark Encrypt Strings (stack) (big)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(65536);
  setup_asherah_static_memory(false, true);
  benchmark_encrypt_string(this.test?.title ?? 'unknown test', get_big_string());
});
it('Benchmark Decrypt Strings (stack) (big)', function () {
  this.timeout(0);
  set_max_stack_alloc_item_size(65536);
  setup_asherah_static_memory(false, true);
  benchmark_decrypt_string(this.test?.title ?? 'unknown test', get_big_string());
});

/*
  it('Benchmark RoundTrip Buffers (heap) (No Session Cache)', function () {
    this.timeout(0);
    set_max_stack_alloc_item_size(0);
    setup_asherah_static_memory(false, false);
    benchmark_roundtrip_buffer(this.test?.title ??  'unknown test');
  });
  it('Benchmark Encrypt Buffers (heap) (No Session Cache)', function () {
    this.timeout(0);
    set_max_stack_alloc_item_size(0);
    setup_asherah_static_memory(false, false);
    benchmark_encrypt_buffer(this.test?.title ??  'unknown test');
  });
  it('Benchmark Decrypt Buffers (heap) (No Session Cache)', function () {
    this.timeout(0);
    set_max_stack_alloc_item_size(0);
    setup_asherah_static_memory(false, false);
    benchmark_decrypt_buffer(this.test?.title ??  'unknown test');
  });
  it('Benchmark RoundTrip Strings (heap) (No Session Cache)', function () {
    this.timeout(0);
    set_max_stack_alloc_item_size(0);
    setup_asherah_static_memory(false, false);
    benchmark_roundtrip_string(this.test?.title ??  'unknown test');
  });
  it('Benchmark Encrypt Strings (heap) (No Session Cache)', function () {
    this.timeout(0);
    set_max_stack_alloc_item_size(0);
    setup_asherah_static_memory(false, false);
    benchmark_encrypt_string(this.test?.title ??  'unknown test');
  });
  it('Benchmark Decrypt Strings (heap) (No Session Cache)', function () {
    this.timeout(0);
    set_max_stack_alloc_item_size(0);
    setup_asherah_static_memory(false, false);
    benchmark_decrypt_string(this.test?.title ??  'unknown test');
  });

  it('Benchmark RoundTrip Buffers (stack) (No Session Cache)', function () {
    this.timeout(0);
    set_max_stack_alloc_item_size(2048);
    setup_asherah_static_memory(false, false);
    benchmark_roundtrip_buffer(this.test?.title ??  'unknown test');
  });
  it('Benchmark Encrypt Buffers (stack) (No Session Cache)', function () {
    this.timeout(0);
    set_max_stack_alloc_item_size(2048);
    setup_asherah_static_memory(false, false);
    benchmark_encrypt_buffer(this.test?.title ??  'unknown test');
  });
  it('Benchmark Decrypt Buffers (stack) (No Session Cache)', function () {
    this.timeout(0);
    set_max_stack_alloc_item_size(2048);
    setup_asherah_static_memory(false, false);
    benchmark_decrypt_buffer(this.test?.title ?? 'unknown test');
  });
  it('Benchmark RoundTrip Strings (stack) (No Session Cache)', function () {
    this.timeout(0);
    set_max_stack_alloc_item_size(2048);
    setup_asherah_static_memory(false, false);
    benchmark_roundtrip_string(this.test?.title ??  'unknown test');
  });
  it('Benchmark Encrypt Strings (stack) (No Session Cache)', function () {
    this.timeout(0);
    set_max_stack_alloc_item_size(2048);
    setup_asherah_static_memory(false, false);
    benchmark_encrypt_string(this.test?.title ??  'unknown test');
  });
  it('Benchmark Decrypt Strings (stack) (No Session Cache)', function () {
    this.timeout(0);
    set_max_stack_alloc_item_size(2048);
    setup_asherah_static_memory(false, false);
    benchmark_decrypt_string(this.test?.title ??  'unknown test');
  });
*/
}
const benchmark_seconds = 3.5;
const benchmark_for_cycles = 1;

function benchmark_roundtrip_buffer(name: string, input: string) {
  try {
    const suite = new Benchmark.Suite(name, { maxTime: 3 }).on('complete', function () {
      const fastest = suite.filter('fastest');
      console.log(name + ' mean ' + (fastest.map('stats')[0]['mean'] * 1000).toFixed(3));
    }).on('error', function (e: any) {
      console.error(e);
    });
    const inputBuffer = Buffer.from(input);
    suite.add('RoundTrip#Buffer', function () {
      for(let i = 0; i < benchmark_for_cycles; i++) {
        const enc = encrypt('partition', inputBuffer);
        decrypt('partition', enc);
      }
    }, { maxTime: benchmark_seconds })
      .run({ 'async': false });
  } finally {
    shutdown();
  }
}

function benchmark_encrypt_buffer(name: string, input: string) {
  try {
    const suite = new Benchmark.Suite(name, { maxTime: 3 }).on('complete', function () {
      const fastest = suite.filter('fastest');
      console.log(name + ' mean ' + (fastest.map('stats')[0]['mean'] * 1000).toFixed(3));
    }).on('error', function (e: any) {
      console.error(e);
    });
    const inputBuffer = Buffer.from(input);
    suite.add('Encrypt#Buffer', function () {
      for(let i = 0; i < benchmark_for_cycles; i++) {
        encrypt('partition', inputBuffer);
      }
    }, { maxTime: benchmark_seconds })
      .run({ 'async': false });
  } finally {
    shutdown();
  }
}

function benchmark_decrypt_buffer(name: string, input: string) {
  try {
    const suite = new Benchmark.Suite(name, { maxTime: 3 }).on('complete', function () {
      const fastest = suite.filter('fastest');
      console.log(name + ' mean ' + (fastest.map('stats')[0]['mean'] * 1000).toFixed(3));
    }).on('error', function (e: any) {
      console.error(e);
    });
    const inputBuffer = Buffer.from(input);
    const enc = encrypt('partition', inputBuffer);
    suite.add('Decrypt#Buffer', function () {
      for(let i = 0; i < benchmark_for_cycles; i++) {
        decrypt('partition', enc);
      }
    }, { maxTime: benchmark_seconds })
      .run({ 'async': false });
  } finally {
    shutdown();
  }
}

function benchmark_roundtrip_string(name: string, input: string) {
  try {
    const suite = new Benchmark.Suite(name, { maxTime: 3 }).on('complete', function () {
      const fastest = suite.filter('fastest');
      console.log(name + ' mean ' + (fastest.map('stats')[0]['mean'] * 1000).toFixed(3));
    }).on('error', function (e: any) {
      console.error(e);
    });
    suite.add('RoundTrip#String', function () {
      for(let i = 0; i < benchmark_for_cycles; i++) {
        const enc = encrypt_string('partition', input);
        decrypt_string('partition', enc);
      }
    }, { maxTime: benchmark_seconds })
      .run({ 'async': false });
  } finally {
    shutdown();
  }
}

function benchmark_encrypt_string(name: string, input: string) {
  try {
    const suite = new Benchmark.Suite(name, { maxTime: 3 }).on('complete', function () {
      const fastest = suite.filter('fastest');
      console.log(name + ' mean ' + (fastest.map('stats')[0]['mean'] * 1000).toFixed(3));
    }).on('error', function (e: any) {
      console.error(e);
    });
    suite.add('Encrypt#String', function () {
      for(let i = 0; i < benchmark_for_cycles; i++) {
        encrypt_string('partition', input);
      }
    }, { maxTime: benchmark_seconds })
    .run({ 'async': false });
  } finally {
    shutdown();
  }
}

function benchmark_decrypt_string(name: string, input: string) {
  try {
    const suite = new Benchmark.Suite(name, { maxTime: 3 }).on('complete', function () {
      const fastest = suite.filter('fastest');
      console.log(name + ' mean ' + (fastest.map('stats')[0]['mean'] * 1000).toFixed(3));
    }).on('error', function (e: any) {
      console.error(e);
    });

    const enc = encrypt_string('partition', input);
    suite.add('Decrypt#String', function () {
      for(let i = 0; i < benchmark_for_cycles; i++) {
        decrypt_string('partition', enc);
      }
    }, { maxTime: benchmark_seconds })
      .run({ 'async': false });
  } finally {
    shutdown();
  }
}

