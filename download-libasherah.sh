#!/bin/bash

rm -rf binaries

wget --content-disposition --directory-prefix binaries/  \
  https://github.com/godaddy/asherah-cobhan/releases/download/current/libasherah-arm64.dylib \
  https://github.com/godaddy/asherah-cobhan/releases/download/current/libasherah-arm64.so \
  https://github.com/godaddy/asherah-cobhan/releases/download/current/libasherah-x64.dylib \
  https://github.com/godaddy/asherah-cobhan/releases/download/current/libasherah-x64.so \
  || exit 1

