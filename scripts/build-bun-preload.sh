#!/bin/bash

# Build script for Bun preload library  
# This creates a minimal C shared library for Bun FFI subsystem initialization

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
PRELOAD_DIR="$PROJECT_DIR/asherah-bun-preload"
LIB_DIR="$PRELOAD_DIR/lib"

echo "Building Bun preload library..."

# Check if clang/gcc is available
if ! command -v clang &> /dev/null && ! command -v gcc &> /dev/null; then
    echo "C compiler not found. Skipping Bun preload build."
    echo "Bun support will not be available until a C compiler is installed."
    exit 0
fi

# Navigate to lib directory  
cd "$LIB_DIR"

# Build the minimal warmup library
echo "Compiling minimal C warmup library..."

# Determine the appropriate file extension and compiler flags for the platform
case "$(uname)" in
    Darwin*)
        LIB_EXT=".dylib"
        COMPILER_FLAGS="-shared -Os -fPIC"
        COMPILER="clang"
        ;;
    Linux*)
        LIB_EXT=".so"
        COMPILER_FLAGS="-shared -Os -fPIC"
        COMPILER="gcc"
        ;;
    CYGWIN*|MINGW*|MSYS*)
        LIB_EXT=".dll"
        COMPILER_FLAGS="-shared -Os"
        COMPILER="gcc"
        ;;
    *)
        echo "Unsupported platform: $(uname)"
        exit 1
        ;;
esac

# Use clang if available, fallback to gcc
if command -v clang &> /dev/null; then
    COMPILER="clang"
elif command -v gcc &> /dev/null; then
    COMPILER="gcc"
fi

# Build the library
$COMPILER $COMPILER_FLAGS -o "noop_warmup$LIB_EXT" noop_warmup.c

echo "✅ Bun preload library built successfully: noop_warmup$LIB_EXT"
echo "📊 Library size: $(du -h "noop_warmup$LIB_EXT" | cut -f1)"