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
cmake -S . -B build-cmake -DBUILD_TESTING=ON
cmake --build build-cmake
ctest --test-dir build-cmake --output-on-failure
```

This configures the project, builds tests and examples, then runs the tests.

## Build without examples

```sh
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
cmake -S . -B build-cmake \
  -DBUILD_TESTING=OFF \
  -DC4_MIXINS_BUILD_EXAMPLES=OFF

cmake --build build-cmake
```

## Test installation of headers

If the `CMakeLists.txt` installs headers with:

```cmake
include(GNUInstallDirs)

install(
  DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
```

then test installation with:

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

## Minimal CMakeLists.txt shape

A minimal useful `CMakeLists.txt` looks like this:

```cmake
cmake_minimum_required(VERSION 3.20)

project(c4-mixins
  VERSION 0.1.0
  DESCRIPTION "Header-only C++20 mixins with a C4-linearized Super chain"
  LANGUAGES CXX)

include(GNUInstallDirs)
include(CTest)

add_library(c4_mixins INTERFACE)
add_library(c4::mixins ALIAS c4_mixins)

target_compile_features(c4_mixins INTERFACE cxx_std_20)

target_include_directories(c4_mixins INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(
  DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

option(C4_MIXINS_BUILD_EXAMPLES "Build c4-mixins examples" ON)

if(BUILD_TESTING)
  set(C4_MIXINS_TESTS
    test_diamond
    test_wrapping
    test_suffix
    test_parent_orders
    test_interface
    test_counting
    test_cycle_check
    test_linearize
  )

  foreach(test_name IN LISTS C4_MIXINS_TESTS)
    add_executable(${test_name} tests/${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE c4::mixins)
    add_test(NAME ${test_name} COMMAND ${test_name})
  endforeach()
endif()

if(C4_MIXINS_BUILD_EXAMPLES)
  set(C4_MIXINS_EXAMPLES
    01_diamond
    02_wrapping
    03_suffix
    04_parent_orders
    05_interface
    06_counting
  )

  foreach(example_name IN LISTS C4_MIXINS_EXAMPLES)
    add_executable(${example_name} examples/${example_name}.cpp)
    target_link_libraries(${example_name} PRIVATE c4::mixins)
  endforeach()
endif()
```

## Troubleshooting

### `install TARGETS given target "c4_mixins" which does not exist`

This means an `install(TARGETS c4_mixins ...)` command appears before:

```cmake
add_library(c4_mixins INTERFACE)
```

or the target has a different name.

For the current minimal setup, you can avoid this entirely by installing only
the headers:

```cmake
install(
  DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
```

### `c4-mixinsConfig.cmake.in does not exist`

That error comes from full package-config generation, usually from a call like:

```cmake
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/c4-mixinsConfig.cmake.in
  ...)
```

The minimal CMake setup does not need package-config generation. Remove that
block unless and until the project supports `find_package(c4-mixins CONFIG
REQUIRED)`.

### Tests pass with `./run-tests.sh` but not with CMake

First confirm CMake is using the same compiler:

```sh
cmake -S . -B build-cmake -DCMAKE_CXX_COMPILER="$(command -v c++)"
```

Then rebuild from a clean directory:

```sh
rm -rf build-cmake
cmake -S . -B build-cmake -DBUILD_TESTING=ON
cmake --build build-cmake
ctest --test-dir build-cmake --output-on-failure
```

## Current recommendation

Use:

```sh
./run-tests.sh
```

for day-to-day development.

Use CMake when you want to check IDE/build-system integration, build all
examples through CMake, run tests through CTest, or verify header installation.
