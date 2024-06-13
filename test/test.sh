#!/bin/sh
set -e

# TODO: this is all pretty rudimentary and manual; the whole testing framework
# should be made more resilient

export TEST_DIR=$(dirname "$0")
export TMP_DIR="/tmp/aylp_test"
export BUILD_DIR="$PWD/build"

mkdir -p "$TMP_DIR"

sh "$TEST_DIR/com.sh"
sh "$TEST_DIR/matmul.sh"

