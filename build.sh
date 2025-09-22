#!/bin/bash
set -euo pipefail  # -u: undefined vars, -o pipefail: detect errors in pipes

# Use Java 8 instead of Java 21
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk
export PATH=$JAVA_HOME/bin:$PATH

# Verify we're using the correct Java version
echo "Using Java version:"
$JAVA_HOME/bin/java -version

# Enable verbose output
set -x

# Clean up
echo "Cleaning up old build files..."
rm -rf build
rm -fr CMakeCache.txt CMakeFiles/ cmake_install.cmake Makefile || true

# Create build directory
mkdir -p build
cd build

# Configure
echo "Configuring..."
cmake -DJAVA_HOME="$JAVA_HOME" \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -G Ninja \
      .. || { echo "CMake configuration failed!"; exit 1; }

# Build using cmake --build (works with any generator)
echo "Building..."
cmake --build . -j"$(nproc)" || { echo "Build failed! Check above output."; exit 1; }

echo "Build completed successfully!"
echo "Verifying library dependencies:"
ldd libphantom.so | grep java
