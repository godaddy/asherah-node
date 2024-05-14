{
  'targets': [
      {
      'target_name': 'asherah',
      'include_dirs': ["<!(node -p \"require('node-addon-api').include_dir\")", "lib/", "src/"],
      "cflags": ["-fexceptions", "-g", "-O3", "-std=c++17", "-fPIC", "-Wno-unknown-pragmas"],
      "cflags_cc": ["-fexceptions", "-g", "-O3", "-std=c++17", "-fPIC", "-Wno-unknown-pragmas"],
      "cflags!": [ "-fno-exceptions"],
      "cflags_cc!": [ "-fno-exceptions" ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'OTHER_CFLAGS': [
          '-fexceptions',
          '-g',
          '-O3',
          '-std=c++17',
          '-fPIC',
          '-Wno-unknown-pragmas'
        ],
      },
      'defines': [ 
        'NAPI_CPP_EXCEPTIONS',
        'NODE_API_SWALLOW_UNTHROWABLE_EXCEPTIONS',
        'NODE_ADDON_API_DISABLE_DEPRECATED',
        'NODE_API_NO_EXTERNAL_BUFFERS_ALLOWED',
        'USE_SCOPED_ALLOCATE_BUFFER',
        ],
      'sources': [
        'src/asherah.cc',
        'src/logging_napi.cc',
        'src/logging_stderr.cc'
      ],
      'libraries': [ '../lib/libasherah.a' ]
    }
  ]
}
