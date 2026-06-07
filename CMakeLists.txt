cmake_minimum_required(VERSION 3.20)

project(c4-mixins
  VERSION 0.1.0
  DESCRIPTION "Header-only C++20 mixins with a C4-linearized Super chain"
  LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CTest)

# Header-only library target.
add_library(c4_mixins INTERFACE)
add_library(c4::mixins ALIAS c4_mixins)

target_compile_features(c4_mixins INTERFACE cxx_std_20)

target_include_directories(c4_mixins INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Tests.
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

# Examples.
option(C4_MIXINS_BUILD_EXAMPLES "Build c4-mixins examples" ON)

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

# Installation.
install(
  DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  TARGETS c4_mixins
  EXPORT c4-mixinsTargets)

install(
  EXPORT c4-mixinsTargets
  NAMESPACE c4::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/c4-mixins)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/c4-mixinsConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/c4-mixinsConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/c4-mixins)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/c4-mixinsConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/c4-mixinsConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/c4-mixinsConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/c4-mixins)
