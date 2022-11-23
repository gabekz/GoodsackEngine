#!/bin/bash

#BUILD_TYPE=$(printf "Release\nDebug" | fzf)
pushd ..
PROJ_DIR=$(pwd)

[[ -f "$PROJ_DIR/CMakeLists.txt" ]] || echo "Failed to find CMakeLists.txt"
[[ -d "$PROJ_DIR/build" ]] || mkdir $PROJ_DIR/build

pushd "$PROJ_DIR/build"

cmake $PROJ_DIR/CMakeLists.txt #-DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
cmake --build .
