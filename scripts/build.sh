#!/bin/bash

set -e

echo "========================================="
echo "Building Polygon Detection Server"
echo "========================================="

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DBUILD_CLIENT=ON

# Build
echo "Building..."
make -j$(nproc)

echo ""
echo "========================================="
echo "Build completed successfully!"
echo "========================================="
echo ""
echo "To run the server:"
echo "  ./build/detector_server --port 9090"
echo ""
echo "To run the client:"
echo "  ./build/detector_client --server 127.0.0.1 --port 9090 --image test.jpg"
echo ""
