#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

if [ ! -f .asherah-version ]; then
  echo "Error: The script is meant to be run from the root directory of the asherah-node project" >&2
  exit 1
fi

# shellcheck disable=SC1091
source .asherah-version

rm -f SHA256SUMS SHA256SUMS-darwin
curl -s -L --fail -O --retry 999 --retry-max-time 0  https://github.com/godaddy/asherah-cobhan/releases/download/${ASHERAH_VERSION}/SHA256SUMS
curl -s -L --fail -O --retry 999 --retry-max-time 0  https://github.com/godaddy/asherah-cobhan/releases/download/${ASHERAH_VERSION}/SHA256SUMS-darwin
