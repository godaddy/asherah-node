#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Check for the existence of the .asherah-version file
if [ ! -f .asherah-version ]; then
  echo "Error: The script is meant to be run from the root directory of the asherah-node project" >&2
  exit 1
fi

# Configure the project using node-gyp
node-gyp configure

# Build the project using node-gyp
node-gyp build

# Create the dist directory if it doesn't exist
mkdir -p dist/

# Copy the asherah.node binary to the dist directory
cp build/Release/asherah.node dist/

# Copy the TypeScript definitions to the dist directory
cp src/asherah.d.ts dist/
