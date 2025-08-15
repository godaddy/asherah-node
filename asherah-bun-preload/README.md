# Bun Runtime Compatibility

This directory contains the minimal Go library required for Bun runtime compatibility with asherah-node.

## Overview

Bun requires Go runtime initialization before loading CGO-based N-API modules. This library provides that initialization through a minimal Go shared library that's loaded via Bun's FFI system.

## Build Process

The library is automatically built during package installation:

```bash
# Automatically runs during npm install
npm run postinstall

# Manual build
npm run prepack

# Clean build artifacts  
npm run clean:bun-preload
```

## Architecture

- **lib/bun_warmup_minimal.go**: Minimal Go source (10 lines)
- **lib/bun_warmup_minimal.dylib**: Compiled shared library (~968KB)
- **scripts/build-bun-preload.sh**: Cross-platform build script
- **scripts/clean-bun-preload.sh**: Cleanup script

## Cross-Platform Support

The build script handles:
- **macOS**: `.dylib` 
- **Linux**: `.so`
- **Windows**: `.dll`

## Requirements

- Go 1.19+ (automatically detected in standard locations)
- Platform-appropriate C compiler for CGO

## How It Works

1. `src/index.js` detects Bun runtime
2. Loads `bun_warmup_minimal.dylib` via `bun:ffi` 
3. Calls `Warmup()` function to initialize Go runtime
4. Proceeds to load main `asherah.node` module

This approach enables full asherah-node functionality in Bun while maintaining Node.js compatibility.