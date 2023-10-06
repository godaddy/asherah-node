{
  'targets': [
      {
      'target_name': 'asherah',
      'include_dirs': ["<!(node -p \"require('node-addon-api').include_dir\")"],
      "cflags": ["-fexceptions", "-g", "-O3", "-std=c++17"],
      "cflags_cc": ["-fexceptions", "-g", "-O3", "-std=c++17"],
      "cflags!": [ "-fno-exceptions"],
      "cflags_cc!": [ "-fno-exceptions" ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'OTHER_CFLAGS': [
          '-fexceptions',
          '-g',
          '-O3',
          '-std=c++17'
        ],
      },
      'defines': [ 'NAPI_CPP_EXCEPTIONS', 'NODE_API_SWALLOW_UNTHROWABLE_EXCEPTIONS', 'NODE_ADDON_API_DISABLE_DEPRECATED', 'NODE_API_NO_EXTERNAL_BUFFERS_ALLOWED' ],
      'sources': [
        'lib/libasherah.h',
        'src/asherah.cc',
        'src/logging.cc',
        'src/logging.h',
        'src/cobhan_napi_interop.cc',
        'src/cobhan_napi_interop.h',
        'src/cobhan.h',
        'src/cobhan.cc',
        'src/hints.h'
      ],
      'libraries': [ '../lib/libasherah.a' ]
    }
  ]
}
