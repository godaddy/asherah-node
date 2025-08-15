#!/usr/bin/env bun

const { dlopen, FFIType, ptr, CString } = require('bun:ffi');

console.log('Testing dlopen from JavaScript...\n');

// First, load the libc to get dlopen/dlsym
const libc = dlopen('/usr/lib/libc.dylib', {
    dlopen: {
        returns: FFIType.ptr,
        args: [FFIType.ptr, FFIType.int]
    },
    dlsym: {
        returns: FFIType.ptr,
        args: [FFIType.ptr, FFIType.ptr]
    },
    dlerror: {
        returns: FFIType.ptr,
        args: []
    }
});

// RTLD flags
const RTLD_LAZY = 0x1;
const RTLD_GLOBAL = 0x8;

// Load our module via dlopen
const modulePath = new CString('./go_setup_full.node');
const handle = libc.symbols.dlopen(modulePath, RTLD_LAZY | RTLD_GLOBAL);

if (!handle) {
    const error = libc.symbols.dlerror();
    console.error('Failed to load module:', error);
    process.exit(1);
}

console.log('Module loaded via dlopen');

// Get SetupJson symbol
const setupJsonName = new CString('SetupJson');
const setupJsonPtr = libc.symbols.dlsym(handle, setupJsonName);

if (!setupJsonPtr) {
    const error = libc.symbols.dlerror();
    console.error('Failed to find SetupJson:', error);
    process.exit(1);
}

console.log('SetupJson found');

// Now we need to call it via FFI
// Define the function signature
const setupJsonLib = dlopen(null, {
    callSetupJson: {
        ptr: setupJsonPtr,
        returns: FFIType.int,
        args: [FFIType.ptr]
    }
});

// Create Cobhan buffer
const config = JSON.stringify({
    ServiceName: 'test',
    ProductID: 'test',
    KMS: 'static',
    Metastore: 'memory'
});

const bytes = new TextEncoder().encode(config);
const buffer = new ArrayBuffer(bytes.length + 8);
const view = new DataView(buffer);
view.setUint32(0, bytes.length, true);
const bufferBytes = new Uint8Array(buffer);
bufferBytes.set(bytes, 8);

console.log('Calling SetupJson via dlopen+FFI...');
try {
    const result = setupJsonLib.symbols.callSetupJson(ptr(bufferBytes));
    console.log('✅ SetupJson returned:', result);
} catch (e) {
    console.error('❌ Failed:', e.message);
}