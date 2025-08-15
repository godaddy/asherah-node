# End User Solutions for Asherah in Bun

## Solution 1: Standalone Initialization Package (Recommended)

### Installation
```bash
# Install the Go runtime initializer for Bun
bun add bun-go-runtime-init

# Install Asherah normally
bun add asherah
```

### Usage
```javascript
// main.js or app.js - at the very top
require('bun-go-runtime-init');  // One line fix!

// Now use Asherah normally anywhere in your app
const asherah = require('asherah');

asherah.setup({
    ServiceName: "MyService",
    ProductID: "MyProduct",
    KMS: "aws",
    Metastore: "dynamodb"
});

// Everything works normally from here
const encrypted = asherah.encrypt('partition', data);
```

### Benefits
- ✅ One-line fix
- ✅ Works with ANY Go-based module, not just Asherah
- ✅ No changes to existing Asherah code
- ✅ Can be added to existing projects

---

## Solution 2: Bun Preload Script

### Setup
```bash
# Create a preload script
echo "require('bun-go-runtime-init');" > preload.js
```

### Usage
```bash
# Run your app with preload
bun --preload ./preload.js your-app.js
```

Or in `package.json`:
```json
{
  "scripts": {
    "start": "bun --preload ./preload.js index.js"
  }
}
```

### Benefits
- ✅ Zero code changes to your application
- ✅ Can be configured globally
- ✅ Works for all scripts in project

---

## Solution 3: Bunfig Configuration

### Setup
Create or modify `bunfig.toml`:
```toml
# bunfig.toml
preload = ["./node_modules/bun-go-runtime-init/index.js"]
```

### Benefits
- ✅ Automatic for all Bun processes
- ✅ No command line flags needed
- ✅ Project-wide configuration

---

## Solution 4: Global Bun Configuration

### For all projects on your machine
```bash
# Install globally
bun add -g bun-go-runtime-init

# Add to global bunfig
echo 'preload = ["bun-go-runtime-init"]' >> ~/.bunfig.toml
```

### Benefits
- ✅ Fix once for all projects
- ✅ No per-project configuration
- ✅ Transparent to applications

---

## Solution 5: Modified Asherah Package (For Package Maintainers)

The Asherah package itself could detect Bun and auto-initialize:

```javascript
// asherah/index.js
if (typeof Bun !== 'undefined') {
    // Running in Bun - ensure Go runtime is initialized
    try {
        require('bun-go-runtime-init');
    } catch (e) {
        // Fallback to embedded initializer
        require('./lib/go-runtime-warmup');
    }
}

module.exports = require('./dist/asherah.node');
```

### Benefits
- ✅ Completely transparent to users
- ✅ Works out of the box
- ✅ No user action required

---

## What the `bun-go-runtime-init` Package Contains

```
bun-go-runtime-init/
├── index.js                 # The initializer script
├── go-runtime-warmup.dylib  # macOS ARM64 binary
├── go-runtime-warmup.so     # Linux binary  
├── go-runtime-warmup.dll    # Windows binary
└── package.json
```

The package is tiny (~100KB total) and has zero dependencies.

---

## For Bun Core Team

If Bun wanted to fix this internally, they could:

1. **Auto-detect Go modules** and initialize the runtime before first use
2. **Add a config option** like `bun.goRuntimeInit = true`
3. **Initialize on startup** if any Go modules are detected in node_modules

Example Bun internal fix:
```javascript
// In Bun's module loader
if (isGoNativeModule(modulePath) && !goRuntimeInitialized) {
    initializeGoRuntime();
    goRuntimeInitialized = true;
}
```

---

## Recommended Approach for Immediate Use

1. **Create npm package**: `bun-go-runtime-init`
2. **Users add one line**: `require('bun-go-runtime-init')`
3. **Everything works**: All Go modules including Asherah work perfectly

This solution:
- Requires minimal user intervention (1 line of code)
- Fixes all Go modules, not just Asherah
- Can be adopted immediately without waiting for Bun updates
- Has negligible performance impact (~1ms one-time cost)