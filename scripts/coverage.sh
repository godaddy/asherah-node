#!/bin/bash

# C++ Code Coverage Script for Asherah-Node

echo "Building with coverage flags..."
export CXXFLAGS="--coverage -fprofile-arcs -ftest-coverage"
export LDFLAGS="--coverage"

# Clean and rebuild
npm run clean 2>/dev/null || true
rm -rf build coverage-report
npm install

echo "Running tests..."
npm test

echo "Generating coverage report..."
mkdir -p coverage-report

# Find all gcda files and generate coverage
if command -v llvm-cov >/dev/null 2>&1; then
    echo "Using llvm-cov..."
    # Use llvm-cov if available
    llvm-cov gcov build/Release/obj.target/asherah/src/*.o
elif command -v gcov >/dev/null 2>&1; then
    echo "Using gcov..."
    # Fall back to gcov
    gcov build/Release/obj.target/asherah/src/*.o
else
    echo "Error: No coverage tool found. Install llvm or gcc."
    exit 1
fi

# Generate HTML report if lcov is available
if command -v lcov >/dev/null 2>&1 && command -v genhtml >/dev/null 2>&1; then
    echo "Generating HTML report..."
    lcov --capture --directory build --output-file coverage-report/coverage.info
    lcov --remove coverage-report/coverage.info '/usr/*' '*/node_modules/*' --output-file coverage-report/coverage.info
    genhtml coverage-report/coverage.info --output-directory coverage-report/html
    echo "HTML report generated in coverage-report/html/index.html"
fi

# Summary
echo ""
echo "Coverage Summary:"
echo "================="
if [ -f "asherah.cc.gcov" ]; then
    grep -E "^ *[0-9]+:" *.gcov | wc -l | xargs echo "Lines executed:"
    grep -E "^ *#####:" *.gcov | wc -l | xargs echo "Lines not executed:"
fi

# Clean up gcov files
mkdir -p coverage-report/gcov
mv *.gcov coverage-report/gcov/ 2>/dev/null || true