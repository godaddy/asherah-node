{
  'targets': [
    {
      'target_name': 'napiasherah',
      'include_dirs': ["<!(node -p \"require('node-addon-api').include_dir\")"],
      'sources': [
        'lib/libasherah.h',
        'src/napiasherah.cc'
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
      'libraries': [ '../lib/libasherah.a' ]
    }
  ]
}
