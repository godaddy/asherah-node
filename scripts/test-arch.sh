#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

echo "Testing ${TEST_TAG} emulation"
docker run --rm -v "$(pwd):/build" --platform "${TEST_PLATFORM}" --entrypoint /build/scripts/runs-on-emulated.sh "${TEST_TAG}/node:bookworm"
