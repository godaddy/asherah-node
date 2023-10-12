#!/bin/bash
cd /build || exit 1
npm install
npm run test
