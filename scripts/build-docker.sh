#!/bin/bash

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "Docker is not installed."
    echo "Please run ./install-docker.sh first, or build natively with ./build.sh"
    exit 1
fi

echo "Building Polygon Detector Docker image..."
docker build -t polygon-detector:latest .

if [ $? -eq 0 ]; then
    echo ""
    echo "========================================="
    echo "Docker build completed successfully!"
    echo "========================================="
    echo ""
    echo "To run the server:"
    echo "  docker run -p 9090:9090 polygon-detector:latest"
    echo ""
    echo "To run in background:"
    echo "  docker run -d -p 9090:9090 --name polygon-detector polygon-detector"
    echo ""
    echo "To stop:"
    echo "  docker stop polygon-detector"
    echo ""
else
    echo "Docker build failed!"
    exit 1
fi
