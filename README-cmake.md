# Using CMake with c4-mixins

The main development command for this repository is still:

```sh
./run-tests.sh
```

The CMake configuration is provided as a convenience for IDEs, users who prefer
CMake, and quick checks that the library can be consumed as a header-only
`INTERFACE` target.

## What CMake currently supports

The current CMake setup is intentionally minimal. It can:

- define the header-only target `c4::mixins`;
- build and run the tests through CTest;
- build the examples;
- optionally install the `include/` directory.

It does **not** currently provide full `find_package(c4-mixins CONFIG REQUIRED)`
package-export support. That can be added later if users need it.

## Configure, build, and test

From the repository root:

```sh
rm -rf build-cmake
cmake -S . -B build-cmake -DBUILD_TESTING=ON
cmake --build build-cmake
ctest --test-dir build-cmake --output-on-failure
```

This configures the project, builds tests and examples, then runs the tests.

## Build without examples

```sh
rm -rf build-cmake
cmake -S . -B build-cmake \
  -DBUILD_TESTING=ON \
  -DC4_MIXINS_BUILD_EXAMPLES=OFF

cmake --build build-cmake
ctest --test-dir build-cmake --output-on-failure
```

## Library-only configure/build

This is useful for checking that the header-only target configures without
building tests or examples:

```sh
rm -rf build-cmake
cmake -S . -B build-cmake \
  -DBUILD_TESTING=OFF \
  -DC4_MIXINS_BUILD_EXAMPLES=OFF

cmake --build build-cmake
```

## Test installation of headers

Test installation of headers with:

```sh
rm -rf build-cmake /tmp/c4-mixins-install

cmake -S . -B build-cmake \
  -DBUILD_TESTING=OFF \
  -DC4_MIXINS_BUILD_EXAMPLES=OFF \
  -DCMAKE_INSTALL_PREFIX=/tmp/c4-mixins-install

cmake --build build-cmake
cmake --install build-cmake
find /tmp/c4-mixins-install -type f
```

You should see installed headers such as:

```text
/tmp/c4-mixins-install/include/c4/mixins.hpp
/tmp/c4-mixins-install/include/c4/type_list.hpp
/tmp/c4-mixins-install/include/c4/type_map.hpp
/tmp/c4-mixins-install/include/c4/cycle_check.hpp
/tmp/c4-mixins-install/include/c4/linearize.hpp
```
