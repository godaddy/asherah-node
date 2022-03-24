#!/bin/bash

echo "Downloading Asherah binaries"

source .asherah-version

rm -rf binaries
mkdir binaries
cd binaries || exit 1

OS=$(uname)
MACHINE=$(uname -m)

if [[ "${OS}" == 'Linux' ]]; then
  if [[ ${MACHINE} == 'x86_64' ]]; then
    echo "Linux x64"
    FILENAME=libasherah-x64.so
    SUMS=../SHA256SUMS
  elif [[ ${MACHINE} == 'aarch64' ]]; then
    echo "Linux arm64"
    FILENAME=libasherah-arm64.so
    SUMS=../SHA256SUMS
  else
    echo "Unsupported CPU architecture"
    exit 1
  fi
elif [[ ${OS} == 'Darwin' ]]; then
  if [[ ${MACHINE} == 'x86_64' ]]; then
    echo "MacOS x64"
    FILENAME=libasherah-x64.dylib
    SUMS=../SHA256SUMS-darwin
  elif [[ ${MACHINE} == 'arm64' ]]; then
    echo "MacOS arm64"
    FILENAME=libasherah-arm64.dylib
    SUMS=../SHA256SUMS-darwin
  else
    echo "Unsupported CPU architecture"
    exit 1
  fi
else
  echo "Unsupported operating system"
  exit 1
fi

curl -s -L --fail -O --retry 999 --retry-max-time 0 "https://github.com/godaddy/asherah-cobhan/releases/download/${ASHERAH_VERSION}/${FILENAME}"
grep "${FILENAME}" "${SUMS}" > ./SHA256SUM
shasum -a 256 -c ./SHA256SUM || (echo 'SHA256 mismatch!' ; rm -f ./*.so ./*.dylib ; exit 1)
