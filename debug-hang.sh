#!/bin/bash
echo "Attaching debugger to find exact hang location..."

# Start Bun in background
/Users/jgowdy/asherah-node/bun/build/debug/bun-debug test-async-proper.js &
PID=$!
sleep 2

# Attach lldb and get backtrace
lldb -p $PID -o "bt all" -o "quit" 2>&1 | head -100