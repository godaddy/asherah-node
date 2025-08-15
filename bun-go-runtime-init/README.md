# bun-go-runtime-init

Fix Go (CGO) native module hangs in Bun with one line of code.

## Problem

Some Go-based native modules (like Asherah, go-sqlite3, etc.) hang when loaded in Bun due to Go runtime initialization conflicts. This package fixes that issue.

## Solution

Add one line to the top of your application:

```javascript
require('bun-go-runtime-init');
```

That's it! Your Go modules will now work perfectly.

## Installation

```bash
bun add bun-go-runtime-init
```

## Usage

### Option 1: In your code (Recommended)
```javascript
// app.js - at the very top
require('bun-go-runtime-init');

// Now use any Go modules normally
const asherah = require('asherah');
const sqlite = require('go-sqlite3');
// ... etc
```

### Option 2: Via bunfig.toml
```toml
# bunfig.toml
preload = ["bun-go-runtime-init"]
```

### Option 3: Via command line
```bash
bun --preload bun-go-runtime-init your-app.js
```

## How It Works

This package pre-initializes the Go runtime with a minimal module before any complex Go modules are loaded, preventing initialization conflicts that cause hangs.

## Compatibility

- ✅ Bun 1.0+
- ✅ macOS (ARM64, x64)
- ✅ Linux (ARM64, x64)
- ✅ Windows (x64)

## FAQ

**Q: Do I need this for Node.js?**  
A: No, this is only needed for Bun. Node.js doesn't have this issue.

**Q: Will this affect performance?**  
A: No, it adds ~1ms one-time startup cost.

**Q: What if I forget to add it?**  
A: Your Go modules may hang. Just add the require statement and restart.

**Q: Can I use it multiple times?**  
A: Yes, it's safe to require multiple times. It only initializes once.

## Troubleshooting

Enable debug output:
```bash
BUN_GO_RUNTIME_DEBUG=1 bun your-app.js
```

## License

MIT

## Credits

Created to fix [Asherah](https://github.com/godaddy/asherah) compatibility with Bun.