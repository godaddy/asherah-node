#!/bin/bash
echo "Running with lldb to see where Go hangs..."
cat > lldb_commands.txt << 'EOF'
b pthread_create
b pthread_key_create
b sigaction
b signal
r test-async-proper.js
c
c
c
bt
EOF

lldb /Users/jgowdy/asherah-node/bun/build/debug/bun-debug -s lldb_commands.txt