#!/bin/bash

rm -rf binaries
mkdir binaries
cd binaries || exit 1
curl -s -L --fail -O --retry 999 --retry-max-time 0  https://github.com/godaddy/asherah-cobhan/releases/download/current/libasherah-arm64.dylib
curl -s -L --fail -O --retry 999 --retry-max-time 0  https://github.com/godaddy/asherah-cobhan/releases/download/current/libasherah-arm64.so
curl -s -L --fail -O --retry 999 --retry-max-time 0  https://github.com/godaddy/asherah-cobhan/releases/download/current/libasherah-x64.dylib
curl -s -L --fail -O --retry 999 --retry-max-time 0  https://github.com/godaddy/asherah-cobhan/releases/download/current/libasherah-x64.so
