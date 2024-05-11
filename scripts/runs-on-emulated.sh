#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

cd /build || exit 1
npm install
npm run test
