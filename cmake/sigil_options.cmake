include_guard(GLOBAL)

option(SIGIL_BUILD_TESTS "Build the sigil test executable" ${PROJECT_IS_TOP_LEVEL})
option(SIGIL_STATIC_RUNTIME "Link the static MSVC runtime (/MT, /MTd) into sigil tests" ON)
option(SIGIL_WARNINGS_AS_ERRORS "Treat compiler warnings as errors for sigil targets" OFF)
set(SIGIL_SANITIZE "" CACHE STRING "Sanitizers for sigil tests: address, undefined or address,undefined (MSVC: address only)")

if(PROJECT_IS_TOP_LEVEL AND SIGIL_STATIC_RUNTIME)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
