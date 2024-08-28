#!/bin/bash

if [ ! -f CMakeLists.txt ]; then
    echo "CMakeLists.txt not found. Please run this script from the root of the project."
    exit 1
fi

BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

cmake -S . -B "$BUILD_DIR"

cmake --build "$BUILD_DIR"
