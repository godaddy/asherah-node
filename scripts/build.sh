#!/bin/bash

set -e

# Check if USE_CMAKE is set to 1
if [ "$USE_CMAKE" = "1" ]; then
    # Run CMake commands
    cmake . && make && mkdir -p dist/ && cp src/asherah.d.ts dist/asherah.d.ts
else
    # Run node-gyp commands
    node-gyp configure && node-gyp build && mkdir -p dist/ && cp build/Release/asherah.node dist/asherah.node && cp src/asherah.d.ts dist/asherah.d.ts
fi
