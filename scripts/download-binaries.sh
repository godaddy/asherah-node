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
    curl -s -L --fail -O --retry 999 --retry-max-time 0  https://github.com/godaddy/asherah-cobhan/releases/download/${ASHERAH_VERSION}/libasherah-x64.so
    shasum -a 256 --ignore-missing -c ../SHA256SUMS || (echo 'SHA256 mismatch!' ; rm -f ./*.so ; exit 1)
  elif [[ ${MACHINE} == 'aarch64' ]]; then
    echo "Linux arm64"
    curl -s -L --fail -O --retry 999 --retry-max-time 0  https://github.com/godaddy/asherah-cobhan/releases/download/${ASHERAH_VERSION}/libasherah-arm64.so
    shasum -a 256 --ignore-missing -c ../SHA256SUMS || (echo 'SHA256 mismatch!' ; rm -f ./*.so ; exit 1)
  else
    echo "Unknown CPU architecture"
    exit 1
  fi
elif [[ ${OS} == 'Darwin' ]]; then
  if [[ ${MACHINE} == 'x86_64' ]]; then
    echo "MacOS x64"
    curl -s -L --fail -O --retry 999 --retry-max-time 0  https://github.com/godaddy/asherah-cobhan/releases/download/${ASHERAH_VERSION}/libasherah-x64.dylib
    shasum -a 256 --ignore-missing -c ../SHA256SUMS-darwin || (echo 'SHA256 mismatch!' ; rm -f ./*.dylib ; exit 1)
  elif [[ ${MACHINE} == 'arm64' ]]; then
    echo "MacOS arm64"
    curl -s -L --fail -O --retry 999 --retry-max-time 0  https://github.com/godaddy/asherah-cobhan/releases/download/${ASHERAH_VERSION}/libasherah-arm64.dylib
    shasum -a 256 --ignore-missing -c ../SHA256SUMS-darwin || (echo 'SHA256 mismatch!' ; rm -f ./*.dylib ; exit 1)
  else
    echo "Unknown CPU architecture"
    exit 1
  fi
else
  echo "Unknown operating system"
  exit 1
fi
