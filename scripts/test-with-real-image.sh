#!/bin/bash

# Скачиваем тестовое изображение
echo "Downloading test image..."
wget -O /home/mary/Projects/gRPS4AleoAlians/build/sample.jpg \
    https://raw.githubusercontent.com/opencv/opencv/master/samples/data/lena.jpg 2>/dev/null || \
    echo "Could not download sample image, using local test.jpg"

if [ -f /home/mary/Projects/gRPS4AleoAlians/build/sample.jpg ]; then
    IMAGE="/home/mary/Projects/gRPS4AleoAlians/build/sample.jpg"
else
    IMAGE="/home/mary/Projects/gRPS4AleoAlians/build/test.jpg"
fi

echo "Testing with image: $IMAGE"
cd /home/mary/Projects/gRPS4AleoAlians/build
./detector_client --server 127.0.0.1 --port 9090 --image "$IMAGE"
