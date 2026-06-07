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
done

echo
echo "== Building expected-failure tests =="
for src in tests/fail_*.cpp; do
  [ -e "$src" ] || continue

  name="$(basename "$src" .cpp)"
  exe="$TEST_BUILD_DIR/$name"
  log="$TEST_BUILD_DIR/$name.log"

  echo "  $name"

  if "$CXX" $CXXFLAGS "$src" -o "$exe" >"$log" 2>&1; then
    echo "ERROR: $src compiled successfully, but it was expected to fail"
    echo "Compiler output:"
    cat "$log"
    exit 1
  fi

  if ! grep -q "C4 linearization failed\|C3 merge failed" "$log"; then
    echo "ERROR: $src failed, but not with an expected C4/C3 diagnostic"
    echo "Compiler output:"
    cat "$log"
    exit 1
  fi
done

echo
echo "== Building examples =="
for src in examples/[0-9][0-9]_*.cpp; do
  name="$(basename "$src" .cpp)"
  exe="$EXAMPLE_BUILD_DIR/$name"

  echo "  $name"
  "$CXX" $CXXFLAGS "$src" -o "$exe"
done

echo
echo "ok"
