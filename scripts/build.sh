#!/bin/bash

set -e

mkdir -p dist/

# Check if USE_CMAKE is set to 1
if [ "$USE_CMAKE" = "1" ]; then
    # Run CMake commands
    cmake . && make
else
    # Run node-gyp commands
    node-gyp configure && node-gyp build && cp build/Release/asherah.node dist/asherah.node
fi

cp src/asherah.d.ts dist/asherah.d.ts
