{
  'targets': [
      {
      'target_name': 'asherah',
      'include_dirs': ["<!(node -p \"require('node-addon-api').include_dir\")"],
      "cflags": ["-fexceptions", "-g"],
      "cflags_cc": ["-fexceptions", "-g"],
      "cflags!": [ "-fno-exceptions"],
      "cflags_cc!": [ "-fno-exceptions" ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'OTHER_CFLAGS': [
          '-fexceptions',
          '-g'
        ],
      },
      'defines': [ 'NAPI_CPP_EXCEPTIONS', 'NODE_API_SWALLOW_UNTHROWABLE_EXCEPTIONS' ],
      'sources': [
        'lib/libasherah.h',
        'src/asherah.cc'
      ],
      'libraries': [ '../lib/libasherah.a' ]
    }
  ]
}
