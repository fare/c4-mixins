#!/usr/bin/env bash
set -euo pipefail

CXX="${CXX:-c++}"
CXXFLAGS="${CXXFLAGS:--std=c++20 -Wall -Wextra -pedantic -Iinclude}"

BUILD_DIR="${BUILD_DIR:-build}"
TEST_BUILD_DIR="$BUILD_DIR/tests"
EXAMPLE_BUILD_DIR="$BUILD_DIR/examples"

mkdir -p "$TEST_BUILD_DIR" "$EXAMPLE_BUILD_DIR"

echo "CXX=$CXX"
echo "CXXFLAGS=$CXXFLAGS"
echo

echo "== Building and running tests =="
for src in tests/test_*.cpp; do
  name="$(basename "$src" .cpp)"
  exe="$TEST_BUILD_DIR/$name"

  echo "  $name"
  "$CXX" $CXXFLAGS "$src" -o "$exe"
  "$exe"
  echo ; echo ; echo
done

echo
echo "== Building examples =="
for src in examples/[0-9][0-9]_*.cpp; do
  name="$(basename "$src" .cpp)"
  exe="$EXAMPLE_BUILD_DIR/$name"

  echo "  $name"
  "$CXX" $CXXFLAGS "$src" -o "$exe"
  echo ; echo ; echo
done

echo
echo "ok"
