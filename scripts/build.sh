#!/bin/bash

node-gyp configure && node-gyp build && mkdir -p dist/ && cp build/Release/asherah.node dist/asherah.node && cp src/asherah.d.ts dist/asherah.d.ts
