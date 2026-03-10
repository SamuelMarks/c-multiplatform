FROM debian:bookworm

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    clang \
    curl \
    libcurl4-openssl-dev \
    git \
    pkg-config \
    libssl-dev \
    sqlite3 \
    libsqlite3-dev \
    valgrind \
    gdb \
    ninja-build \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
