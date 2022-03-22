#!/bin/bash

source .asherah-version

mkdir -p dist
cd dist || exit 1
rm -f SHA256SUMS SHA256SUMS-darwin
curl -s -L --fail -O --retry 999 --retry-max-time 0  https://github.com/godaddy/asherah-cobhan/releases/download/${ASHERAH_VERSION}/SHA256SUMS
curl -s -L --fail -O --retry 999 --retry-max-time 0  https://github.com/godaddy/asherah-cobhan/releases/download/${ASHERAH_VERSION}/SHA256SUMS-darwin
