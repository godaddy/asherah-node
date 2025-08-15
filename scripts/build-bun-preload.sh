#!/bin/bash

# Build script for Bun preload library
# This creates a minimal Go shared library for Bun runtime initialization

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
PRELOAD_DIR="$PROJECT_DIR/asherah-bun-preload"
LIB_DIR="$PRELOAD_DIR/lib"

echo "Building Bun preload library..."

# Check if Go is available and find the best Go binary
GO_BINARY=""
if command -v go &> /dev/null; then
    GO_BINARY="go"
elif command -v /opt/homebrew/bin/go &> /dev/null; then
    GO_BINARY="/opt/homebrew/bin/go"
elif command -v /usr/local/bin/go &> /dev/null; then
    GO_BINARY="/usr/local/bin/go"
else
    echo "Go not found. Skipping Bun preload build."
    echo "Bun support will not be available until Go is installed."
    exit 0
fi

echo "Using Go binary: $GO_BINARY"

# Navigate to lib directory
cd "$LIB_DIR"

# Build the minimal warmup library
echo "Compiling minimal Go warmup library..."

# Determine the appropriate file extension for the platform
case "$(uname)" in
    Darwin*)
        LIB_EXT=".dylib"
        BUILD_FLAGS="-buildmode=c-shared"
        ;;
    Linux*)
        LIB_EXT=".so"
        BUILD_FLAGS="-buildmode=c-shared"
        ;;
    CYGWIN*|MINGW*|MSYS*)
        LIB_EXT=".dll"
        BUILD_FLAGS="-buildmode=c-shared"
        ;;
    *)
        echo "Unsupported platform: $(uname)"
        exit 1
        ;;
esac

# Build the library
$GO_BINARY build $BUILD_FLAGS -o "bun_warmup_minimal$LIB_EXT" bun_warmup_minimal.go

# Remove the header file (not needed for FFI)
rm -f bun_warmup_minimal.h

echo "✅ Bun preload library built successfully: bun_warmup_minimal$LIB_EXT"