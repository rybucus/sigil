cmake_minimum_required(VERSION 3.28)

project(sigil
  VERSION 0.1.0
  DESCRIPTION "Header-only compile-time byte pattern parser with an AVX2 scanner"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(sigil_options)
include(sigil_compiler_flags)
include(sigil_dependencies)

set(SIGIL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

add_library(sigil INTERFACE)
add_library(sigil::sigil ALIAS sigil)
target_compile_features(sigil INTERFACE cxx_std_23)
target_compile_options(sigil INTERFACE $<$<NOT:$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>:-mavx2>)
target_include_directories(sigil INTERFACE $<BUILD_INTERFACE:${SIGIL_INCLUDE_DIR}>)
target_sources(sigil INTERFACE
  FILE_SET HEADERS
  BASE_DIRS ${SIGIL_INCLUDE_DIR}
  FILES
    ${SIGIL_INCLUDE_DIR}/sigil/algorithm.hxx
    ${SIGIL_INCLUDE_DIR}/sigil/bits.hxx
    ${SIGIL_INCLUDE_DIR}/sigil/concepts.hxx
    ${SIGIL_INCLUDE_DIR}/sigil/const_vector.hxx
    ${SIGIL_INCLUDE_DIR}/sigil/fixed_string.hxx
    ${SIGIL_INCLUDE_DIR}/sigil/optional.hxx
    ${SIGIL_INCLUDE_DIR}/sigil/pattern.hxx
    ${SIGIL_INCLUDE_DIR}/sigil/platform.hxx
    ${SIGIL_INCLUDE_DIR}/sigil/scanner.hxx
    ${SIGIL_INCLUDE_DIR}/sigil/sigil.hxx
    ${SIGIL_INCLUDE_DIR}/sigil/traits.hxx)

if(SIGIL_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
