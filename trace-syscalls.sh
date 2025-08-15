#!/bin/bash
echo "Tracing system calls to see where Go hangs..."
# Note: requires sudo on macOS
sudo dtruss -f /Users/jgowdy/asherah-node/bun/build/debug/bun-debug test-async-proper.js 2>&1 | head -200