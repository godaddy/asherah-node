{
  "targets": [
    {
      "target_name": "test_library_entry",
      "sources": ["test-library-entrypoint.cc"],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "lib"
      ],
      "libraries": [
        "<(module_root_dir)/lib/libasherah.a"
      ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.7",
        "OTHER_LDFLAGS": [
          "-Wl,-rpath,@loader_path"
        ]
      },
      "msvs_settings": {
        "VCCLCompilerTool": { "ExceptionHandling": 1 }
      }
    }
  ]
}