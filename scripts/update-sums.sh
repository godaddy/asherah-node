#!/bin/bash

set -e

source .asherah-version

rm -f SHA256SUMS SHA256SUMS-darwin
curl -sS -L --fail --retry 3 --retry-delay 2 --max-time 60 -O "https://github.com/godaddy/asherah-cobhan/releases/download/${ASHERAH_VERSION}/SHA256SUMS"
curl -sS -L --fail --retry 3 --retry-delay 2 --max-time 60 -O "https://github.com/godaddy/asherah-cobhan/releases/download/${ASHERAH_VERSION}/SHA256SUMS-darwin"
