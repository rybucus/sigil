include_guard(GLOBAL)

include(FetchContent)

if(SIGIL_BUILD_TESTS)
  FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    GIT_SHALLOW TRUE)
  if(SIGIL_STATIC_RUNTIME)
    set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
  else()
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  endif()
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()
