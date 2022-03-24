#!/bin/bash

echo "Downloading Asherah libraries"

source .asherah-version

rm -rf lib
mkdir lib
cd lib || exit 1

OS=$(uname)
MACHINE=$(uname -m)

if [[ "${OS}" == 'Linux' ]]; then
  if [[ ${MACHINE} == 'x86_64' ]]; then
    echo "Linux x64"
    ARCHIVE=libasherah-x64.a
    HEADER=libasherah-x64-archive.h
    SUMS=../SHA256SUMS
  elif [[ ${MACHINE} == 'aarch64' ]]; then
    echo "Linux arm64"
    ARCHIVE=libasherah-arm64.a
    HEADER=libasherah-arm64-archive.h
    SUMS=../SHA256SUMS
  else
    echo "Unsupported CPU architecture"
    exit 1
  fi
elif [[ ${OS} == 'Darwin' ]]; then
  if [[ ${MACHINE} == 'x86_64' ]]; then
    echo "MacOS x64"
    ARCHIVE=libasherah-darwin-x64.a
    HEADER=libasherah-darwin-x64-archive.h
    SUMS=../SHA256SUMS-darwin
  elif [[ ${MACHINE} == 'arm64' ]]; then
    echo "MacOS arm64"
    ARCHIVE=libasherah-darwin-arm64.a
    HEADER=libasherah-darwin-arm64-archive.h
    SUMS=../SHA256SUMS-darwin
  else
    echo "Unsupported CPU architecture"
    exit 1
  fi
else
  echo "Unsupported operating system"
  exit 1
fi

curl -s -L --fail -O --retry 999 --retry-max-time 0 "https://github.com/godaddy/asherah-cobhan/releases/download/${ASHERAH_VERSION}/${ARCHIVE}" || echo "Failed to download ${ASHERAH_VERSION}/${ARCHIVE}"
curl -s -L --fail -O --retry 999 --retry-max-time 0 "https://github.com/godaddy/asherah-cobhan/releases/download/${ASHERAH_VERSION}/${HEADER}" || echo "Failed to download ${ASHERAH_VERSION}/${HEADER}"
grep -e "${ARCHIVE}" -e "${HEADER}" "${SUMS}" > ./SHA256SUM
#shasum -a 256 -c ./SHA256SUM || (echo 'SHA256 mismatch!' ; rm -f ./*.a ./*.h ; exit 1)

mv "${ARCHIVE}" libasherah.a
mv "${HEADER}" libasherah.h
