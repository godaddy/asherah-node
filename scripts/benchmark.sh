#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Compile TypeScript code to JavaScript using the TypeScript Compiler (tsc)
echo "Starting TypeScript compilation..."
tsc

# If tsc fails, the script will exit due to 'set -e', and the following lines will not execute.
echo "Compilation successful. Running benchmarks..."

# Run the compiled JavaScript with Node.js
node dist/benchmarks.js
