# Dynamic AWS SDK Loading Analysis

## Problem Statement
The current asherah-cobhan library statically links AWS SDK dependencies, causing crypto/x509/internal/macos initialization that triggers SIGBUS in Bun runtime. We need to explore if AWS SDK can be loaded dynamically.

## Current Architecture
- **Static Linking**: All AWS SDK dependencies compiled into libasherah.a
- **Package Init Chain**: AWS SDK → crypto/tls → crypto/x509 → macOS system calls
- **Result**: Crypto init happens during library load, before any Go functions called

## Dynamic Loading Approaches

### 1. Go Plugin System
**Mechanism**: Compile AWS components as Go plugins (.so files) loaded at runtime
**Viability**: ❌ **Not Viable**
- Plugins only supported on Linux/Darwin/FreeBSD (no Windows)  
- Requires exact same Go version, build tags, environment variables
- All components must be built together by same toolchain
- Runtime crashes likely with version mismatches
- Adds significant complexity and fragility

### 2. Custom asherah-cobhan Builds
**Mechanism**: Modify asherah-cobhan to support build tags excluding AWS
**Status**: 🔍 **Investigating**
- No existing build tags found in asherah-cobhan source
- Would require upstream changes to support conditional AWS compilation
- Current architecture assumes AWS SDK always present

### 3. FFI-Based Dynamic Loading
**Mechanism**: Load AWS SDK as separate shared library via FFI
**Challenges**: 
- Go packages can't be "unlinked" once imported
- Package init() functions run during import, not function call
- Would need complete architectural restructure

### 4. Multi-Binary Architecture
**Mechanism**: Separate binaries for AWS and non-AWS functionality
**Viability**: 🤔 **Possible but Complex**
- Main binary: Static KMS + memory metastore
- AWS binary: Full AWS SDK functionality  
- IPC/RPC communication between binaries
- Significant architectural changes required

## Technical Analysis

### Current Dependency Chain (from symbol analysis):
```
libasherah.a contains:
├── aws-v1/kms (AWS KMS plugin)
├── aws-v1/dynamodb (DynamoDB plugin)  
├── aws-sdk-go (v1, being deprecated 2025)
├── crypto/tls (TLS support)
└── crypto/x509 (Certificate handling → macOS system calls)
```

### AWS SDK Migration Context:
- AWS SDK Go v1 reaches end-of-support July 31, 2025
- Migration to AWS SDK Go v2 required
- v2 has different architecture but likely same crypto dependencies

## Recommendations

### Option A: Upstream asherah-cobhan Changes
1. **Add build tags** to exclude AWS SDK dependencies
2. **Create AWS-free build** with only static KMS + memory metastore
3. **Separate AWS plugin** loaded conditionally

**Pros**: Clean architecture, upstream benefits
**Cons**: Requires upstream coordination, significant work

### Option B: Custom Fork Approach  
1. **Fork asherah-cobhan** 
2. **Strip AWS dependencies** for Bun-compatible build
3. **Maintain parallel builds** (AWS + non-AWS)

**Pros**: Full control, faster implementation
**Cons**: Maintenance burden, divergence from upstream

### Option C: Proxy Service Architecture
1. **Lightweight Bun-compatible library** (current approach)
2. **Separate AWS service** running in Node.js for full functionality
3. **Service discovery/routing** between runtimes

**Pros**: Runtime isolation, leverages existing solutions
**Cons**: Deployment complexity, latency overhead

## Current Working Solution Assessment
Our existing approach (separate minimal Go library for Bun warmup) is:
- ✅ **Functional**: Works reliably in both Node.js and Bun
- ✅ **Simple**: Minimal changes to existing codebase  
- ✅ **Maintainable**: No dependency on upstream changes
- ⚠️ **Limited**: Only basic crypto operations, no AWS features in Bun

## Conclusion
Dynamic AWS SDK loading faces fundamental limitations in Go's package system. The most viable path for full AWS functionality in Bun would require upstream changes to asherah-cobhan or maintaining a custom fork, both significant undertakings.

Given the complexity and risks, the current unified integration with separate minimal library remains the optimal solution for production use.