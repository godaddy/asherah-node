{
  'targets': [
      {
      'target_name': 'asherah',
      'include_dirs': ["<!(node -p \"require('node-addon-api').include_dir\")"],
      "cflags": ["-fexceptions", "-g", "-O3"],
      "cflags_cc": ["-fexceptions", "-g", "-O3"],
      "cflags!": [ "-fno-exceptions"],
      "cflags_cc!": [ "-fno-exceptions" ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'OTHER_CFLAGS': [
          '-fexceptions',
          '-g',
          '-O3'
        ],
      },
      'defines': [ 'NAPI_CPP_EXCEPTIONS', 'NODE_API_SWALLOW_UNTHROWABLE_EXCEPTIONS', 'NODE_ADDON_API_DISABLE_DEPRECATED' ],
      'sources': [
        'lib/libasherah.h',
        'src/asherah.cc',
        'src/logging.cc',
        'src/logging.h',
        'src/cobhan_napi_interop.cc',
        'src/cobhan_napi_interop.h',
        'src/hints.h'
      ],
      'libraries': [ '../lib/libasherah.a' ]
    }
  ]
}
