#!/bin/bash
cd /build || exit 1

# Install Bun for multi-arch testing
echo "Installing Bun..."
curl -fsSL https://bun.sh/install | bash
export PATH="$HOME/.bun/bin:$PATH"
bun --version

# Run npm install and tests
npm install
npm run test
