FROM alpine:latest

RUN apk add --no-cache \
    build-base \
    cmake \
    clang \
    curl \
    curl-dev \
    git \
    pkgconf \
    openssl-dev \
    sqlite \
    sqlite-dev \
    valgrind \
    gdb \
    ninja \
    linux-headers

WORKDIR /workspace
