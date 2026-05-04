#!/bin/bash

cd /home/mary/Projects/gRPS4AleoAlians

# Check if build exists
if [ ! -f build/detector_server ]; then
    echo "Server not built. Running build.sh first..."
    ./build.sh
fi

echo "========================================="
echo "Starting Polygon Detection Server"
echo "========================================="
echo "Server will listen on port 9090"
echo "Press Ctrl+C to stop"
echo ""

./build/detector_server --port 9090
