# Bun Runtime Compatibility Proposal for asherah-cobhan

## Problem Statement

The asherah-node package uses a CGO-based N-API module that crashes in Bun runtime due to Go runtime initialization issues. Specifically, Bun doesn't initialize the Go runtime before loading CGO modules, causing SIGBUS errors when crypto/x509 initialization runs.

## Solution

Add a minimal Go warmup library to asherah-cobhan that can be loaded via Bun's FFI to initialize the Go runtime before loading the main asherah library.

## Implementation

### 1. Add to asherah-cobhan

Create `bun_warmup_minimal.go`:

```go
package main

import "C"

//export Warmup
func Warmup() C.int {
    return 1
}

func main() {}
```

### 2. Build Process

Add to asherah-cobhan's build script to create platform-specific libraries:

- **Linux x64**: `bun_warmup_minimal-linux-x64.so`
- **Linux ARM64**: `bun_warmup_minimal-linux-arm64.so`
- **Darwin x64**: `bun_warmup_minimal-darwin-x64.dylib`
- **Darwin ARM64**: `bun_warmup_minimal-darwin-arm64.dylib`

Build command:
```bash
go build -buildmode=c-shared -o bun_warmup_minimal-${PLATFORM}-${ARCH}.${EXT} bun_warmup_minimal.go
```

### 3. Include in Releases

Add these warmup libraries to GitHub releases alongside existing libasherah-*.a files.

### 4. Update SHA256SUMS

Include checksums for the warmup libraries in the existing checksum files.

## Benefits

1. **No Go required at npm install time** - Libraries are pre-built
2. **Consistent with existing architecture** - Uses same release/download mechanism
3. **Platform-specific support** - Matches existing per-architecture approach
4. **Minimal size overhead** - ~968KB per platform
5. **Backward compatible** - asherah-node can gracefully handle missing warmup libraries in older releases

## Usage in asherah-node

asherah-node will:
1. Download warmup libraries via existing `download-libraries.sh`
2. Auto-detect Bun runtime
3. Load appropriate warmup library via Bun FFI
4. Call `Warmup()` to initialize Go runtime
5. Proceed to load main asherah.node module

## Testing

The warmup library has been tested and confirmed to fix the Bun compatibility issue, enabling full encrypt/decrypt functionality in Bun runtime.