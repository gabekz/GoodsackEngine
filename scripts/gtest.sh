#!/bin/bash

pushd ..
PROJ_DIR=$(pwd)
COMPILE_PATH=$(echo "$PROJ_DIR/scripts/compile.sh")

TEST_RUNNER=$(echo "$PROJ_DIR/build/testRunner")

[[ -f $TEST_RUNNER ]] && (exec $TEST_RUNNER || true)

# Create coverage report
[[ -d "$PROJ_DIR/cache" ]] || mkdir $(echo $PROJ_DIR/cache)
lcov --capture --directory . --output-file cache/coverage.info
lcov --remove cache/coverage.info "/usr*" -o cache/coverage.info
lcov --remove cache/coverage.info "*lib/*" -o cache/coverage.info
genhtml cache/coverage.info --output-directory cache/HTML
