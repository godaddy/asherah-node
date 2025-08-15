# Go Warmup Library Integration Proposal

## Problem Statement

The asherah-node package uses a CGO-based N-API module that crashes in Bun runtime due to Go runtime initialization issues. Specifically, Bun doesn't initialize the Go runtime before loading CGO modules, causing SIGBUS errors when crypto/x509 initialization runs.

## Solution

Add a minimal Go warmup library to asherah-cobhan that can be loaded via FFI from JavaScript runtimes to initialize the Go runtime before loading the main asherah library.

## Implementation

### 1. Add to asherah-cobhan

Create `go_warmup.go`:

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

- **Linux x64**: `go-warmup-linux-x64.so`
- **Linux ARM64**: `go-warmup-linux-arm64.so`
- **Darwin x64**: `go-warmup-darwin-x64.dylib`
- **Darwin ARM64**: `go-warmup-darwin-arm64.dylib`

Build command:
```bash
go build -buildmode=c-shared -o go-warmup-${PLATFORM}-${ARCH}.${EXT} go_warmup.go
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
6. **Runtime agnostic** - Works for any JavaScript runtime that needs Go initialization

## Usage in asherah-node

asherah-node will:
1. Download warmup libraries via existing `download-libraries.sh`
2. Auto-detect JavaScript runtime (e.g., Bun)
3. Load appropriate warmup library via FFI
4. Call `Warmup()` to initialize Go runtime
5. Proceed to load main asherah.node module

## Testing

The warmup library has been tested and confirmed to fix JavaScript runtime compatibility issues, enabling full encrypt/decrypt functionality in runtimes like Bun that don't properly initialize the Go runtime.