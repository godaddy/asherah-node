# Remaining Issues Report - Asherah Node

After implementing fixes across multiple branches:
- `add-lto-optimization` - Link-Time Optimization
- `add-secure-memory-wiping` - Secure memory wiping for sensitive data
- `fix-exception-handling` - Exception handling in async workers
- `add-jsdocs` - Comprehensive JSDoc documentation
- `fix-empty-partition-validation` - Empty partition ID validation
- `optimize-partition-validation` - String length optimization

## Critical Issues (3 total)

### 1. Integer Overflow in Size Calculations
**Severity**: High  
**Location**: `src/asherah.cc:437`
```cpp
auto new_size = (size_t)item_size.Int32Value();
```
**Issue**: Negative int32 values will wrap to huge size_t values  
**Fix**: Add bounds checking before cast

### 2. Potential Integer Overflow in EstimateAsherahOutputSize
**Severity**: High  
**Location**: `src/asherah.cc:753-756`
```cpp
size_t est_data_byte_len =
    size_t(double(data_byte_len + est_encryption_overhead) *
           base64_overhead) + 1;
```
**Issue**: Large inputs could overflow when multiplied by 1.34  
**Fix**: Add SIZE_MAX/2 check before calculation

### 3. Buffer Underflow in AllocationSizeToMaxDataSize
**Severity**: High  
**Location**: `src/cobhan_buffer.h:96-104`
```cpp
size_t data_len_bytes = allocation_len_bytes - cobhan_header_size_bytes -
                        canary_size_bytes - safety_padding_bytes;
```
**Issue**: No underflow check if allocation_len_bytes is too small  
**Fix**: Check allocation_len_bytes >= total_overhead before subtraction

## Medium Priority Issues

### 5. Missing Error Context in Cobhan Buffer Errors
**Severity**: Low  
**Location**: Multiple locations in `cobhan_buffer.h`
**Issue**: Generic error messages don't indicate which operation failed  
**Fix**: Add context to error messages (e.g., "CobhanBuffer constructor: allocation size too small")

### 6. Inconsistent Error Handling Pattern
**Severity**: Low  
**Location**: `cobhan_buffer_napi.h:105-106`
```cpp
napi_throw_error(env, nullptr,
                 "Failed to create Napi::String from CobhanBuffer");
```
**Issue**: Uses `napi_throw_error` directly instead of `NapiUtils::ThrowException`  
**Fix**: Use consistent error throwing mechanism

### 7. Missing Validation for Config JSON Properties
**Severity**: Low  
**Location**: `src/asherah.cc:BeginSetupAsherah`
**Issue**: No validation that required config properties exist  
**Fix**: Add checks for required properties before use

### 8. Uninitialized partition_id_length in Some Code Paths
**Severity**: Low  
**Location**: `src/asherah.cc` (DecryptSync methods without stack allocation)
**Issue**: Variable used before initialization in some paths  
**Fix**: Ensure consistent initialization

## Performance Optimization Opportunities

### 9. Redundant Buffer Allocations
**Severity**: Performance  
**Location**: Throughout async operations
**Issue**: Buffers are allocated and copied multiple times  
**Opportunity**: Consider buffer pooling or reuse strategy

### 10. Missing Move Semantics in Some Cases
**Severity**: Performance  
**Location**: `CobhanBufferNapi` usage patterns
**Issue**: Some operations copy when they could move  
**Opportunity**: Add more move constructors/operators

### 11. Inefficient String Conversions
**Severity**: Performance  
**Location**: JSON parsing/stringification
**Issue**: Multiple round trips between C++ and JavaScript for JSON  
**Opportunity**: Consider native JSON parsing library

## Testing Gaps

### 12. No Tests for Maximum Size Limits
**Severity**: Testing  
**Issue**: No tests verify behavior at size boundaries  
**Fix**: Add tests for max buffer sizes, partition ID lengths

### 13. No Stress Tests
**Severity**: Testing  
**Issue**: No tests for concurrent operations or memory pressure  
**Fix**: Add stress tests for async operations

### 14. Missing Negative Test Cases
**Severity**: Testing  
**Issue**: Limited testing of error paths  
**Fix**: Add tests for all error conditions

## API Design Issues

### 15. Inconsistent Parameter Validation
**Severity**: API Design  
**Location**: Various public methods
**Issue**: Some methods validate parameters, others don't  
**Fix**: Add consistent parameter validation at API boundaries

### 16. No Way to Configure Buffer Allocation Strategy
**Severity**: API Design  
**Issue**: Stack allocation size is global, not per-operation  
**Fix**: Consider per-operation allocation hints

## Documentation Issues

### 17. Missing Architecture Documentation
**Severity**: Documentation  
**Issue**: No high-level architecture docs explaining Cobhan protocol  
**Fix**: Add architecture.md explaining the design

### 18. No Performance Tuning Guide
**Severity**: Documentation  
**Issue**: No guidance on optimal buffer sizes or allocation strategies  
**Fix**: Add performance tuning documentation

## Build System Issues

### 19. No Debug Symbol Generation in Release Builds
**Severity**: Build  
**Location**: `binding.gyp`
**Issue**: Hard to debug production issues  
**Fix**: Add symbol generation with separate symbol files

### 20. Missing Compiler Security Flags
**Severity**: Build/Security  
**Issue**: Not using -fstack-protector-strong, -D_FORTIFY_SOURCE=2  
**Fix**: Add security hardening flags

## Summary

The codebase is well-structured with good memory safety practices. The main issues are:
1. Integer overflow scenarios in size calculations
2. Missing bounds checking in a few places
3. Performance optimization opportunities
4. Testing gaps for edge cases

Most issues are straightforward to fix without architectural changes. The use of RAII and defensive programming (canary values) shows good engineering practices.