#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

if [ ! -f .asherah-version ]; then
  echo "Error: The script is meant to be run from the root directory of the asherah-node project" >&2
  exit 1
fi

ORIG_DIR=$(pwd)

git -C "${ORIG_DIR}/integration" clone https://github.com/godaddy/asherah.git
cd "${ORIG_DIR}/integration/asherah" || exit 1

GO_DIR="${ORIG_DIR}/integration/go"
mkdir -p "${GO_DIR}/bin"
export GOPATH=${GO_DIR}
export GOBIN=${GO_DIR}/bin
PATH=$GOPATH/bin:$GOROOT/bin:$PATH

### Encrypt with Go
cd "${ORIG_DIR}/integration/asherah/tests/cross-language/go" || exit 1
go mod edit -replace github.com/godaddy/asherah/go/appencryption=../../../go/appencryption
go install github.com/cucumber/godog/cmd/godog@v0.12.6
echo "GOPATH: ${GOPATH}"
echo "PATH: ${PATH}"
echo "GOBIN: ${GOBIN}"
./scripts/build.sh
go mod tidy

echo Encrypt with Go
godog "${ORIG_DIR}/integration/node/features/encrypt.feature"

echo Encrypt and Decrypt with Asherah-Node

cd "${ORIG_DIR}/integration/node" || exit 1
echo Installing npm packages
CXXFLAGS='-DNODE_API_EXPERIMENTAL_NOGC_ENV_OPT_OUT' npm install

echo "Running cucumber-js encrypt.feature"
./node_modules/.bin/cucumber-js ./features/encrypt.feature

### Decrypt with Asherah-Node

echo "Running cucumber-js decrypt.feature"
./node_modules/.bin/cucumber-js ./features/decrypt.feature

### Decrypt with Go

echo Decrypt with Go
cd "${ORIG_DIR}/integration/asherah/tests/cross-language/go" || exit 1
godog "${ORIG_DIR}/integration/node/features/decrypt.feature"
