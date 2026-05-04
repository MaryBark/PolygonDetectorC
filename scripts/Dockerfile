FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    libjson-c-dev \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source code
COPY . .

# Build project
RUN mkdir build && cd build && \
    cmake .. -DBUILD_CLIENT=ON && \
    make -j$(nproc)

# Expose port
EXPOSE 9090

# Run server
CMD ["./build/detector_server", "--port", "9090"]
