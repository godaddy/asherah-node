#!/bin/bash
cd /build || exit 1

# Install Bun for multi-arch testing
echo "Installing Bun..."
# In Docker containers, HOME is usually /root
export BUN_INSTALL="/root/.bun"
curl -fsSL https://bun.sh/install | bash

# Make Bun available for npm scripts
mkdir -p /usr/local/bin
cp /root/.bun/bin/bun /usr/local/bin/bun
chmod +x /usr/local/bin/bun
echo "Bun installed and copied to /usr/local/bin"
/usr/local/bin/bun --version

# Run npm install and tests
npm install
npm run test
