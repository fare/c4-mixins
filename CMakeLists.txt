cmake_minimum_required(VERSION 3.20)

project(c4-mixins
  VERSION 0.1.1
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

install(
  TARGETS c4_mixins
  EXPORT c4_mixinsTargets)

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
    test_gerbil_vectors
  )

  foreach(test_name IN LISTS C4_MIXINS_TESTS)
    add_executable(${test_name} tests/${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE c4::mixins)
    add_test(NAME ${test_name} COMMAND ${test_name})
  endforeach()

  set(C4_MIXINS_FAIL_TESTS
    fail_confused_grid
    fail_suffix_break
    fail_parent_order_cycle
  )

  foreach(test_name IN LISTS C4_MIXINS_FAIL_TESTS)
    add_test(
      NAME ${test_name}
      COMMAND
        ${CMAKE_CXX_COMPILER}
        -std=c++20
        -I${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/${test_name}.cpp
        -o ${CMAKE_CURRENT_BINARY_DIR}/${test_name})

    set_tests_properties(${test_name}
      PROPERTIES
        WILL_FAIL TRUE)
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
