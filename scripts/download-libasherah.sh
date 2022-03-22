#!/bin/bash

VERSION=v0.4.3

rm -rf binaries
mkdir binaries
cd binaries || exit 1
curl -s -L --fail -O --retry 999 --retry-max-time 0  https://github.com/godaddy/asherah-cobhan/releases/download/${VERSION}/libasherah-arm64.dylib
sha256sum libasherah-arm64.dylib
curl -s -L --fail -O --retry 999 --retry-max-time 0  https://github.com/godaddy/asherah-cobhan/releases/download/${VERSION}/libasherah-arm64.so
sha256sum libasherah-arm64.so
curl -s -L --fail -O --retry 999 --retry-max-time 0  https://github.com/godaddy/asherah-cobhan/releases/download/${VERSION}/libasherah-x64.dylib
sha256sum libasherah-x64.dylib
curl -s -L --fail -O --retry 999 --retry-max-time 0  https://github.com/godaddy/asherah-cobhan/releases/download/${VERSION}/libasherah-x64.so
sha256sum libasherah-x64.so
