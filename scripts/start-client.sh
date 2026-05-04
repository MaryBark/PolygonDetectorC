#!/bin/bash

cd /home/mary/Projects/gRPS4AleoAlians/build

# Create test image if not exists
if [ ! -f test.jpg ]; then
    echo "Creating test image..."
    dd if=/dev/zero of=test.jpg bs=1024 count=100 2>/dev/null
    echo "Test image created: test.jpg"
fi

# Check if server is running
if ! nc -z 127.0.0.1 9090 2>/dev/null; then
    echo "WARNING: Server does not seem to be running on port 9090"
    echo "Please start the server first: ./start-server.sh"
    echo ""
    read -p "Continue anyway? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

echo "========================================="
echo "Starting Polygon Detection Client"
echo "========================================="
echo "Server: 127.0.0.1:9090"
echo "Image: test.jpg"
echo ""

./detector_client --server 127.0.0.1 --port 9090 --image test.jpg

echo ""
echo "Client finished."
