include_guard(GLOBAL)

add_library(sigil_compiler_flags INTERFACE)
add_library(sigil::compiler_flags ALIAS sigil_compiler_flags)

target_compile_features(sigil_compiler_flags INTERFACE cxx_std_23)

if(MSVC)
  target_compile_options(sigil_compiler_flags INTERFACE
    /W4
    /permissive-
    /utf-8
    /EHsc
    /Zc:__cplusplus
    /Zc:preprocessor
    /Zc:inline
    /MP
    $<$<BOOL:${SIGIL_WARNINGS_AS_ERRORS}>:/WX>)
else()
  target_compile_options(sigil_compiler_flags INTERFACE
    -Wall
    -Wextra
    $<$<BOOL:${SIGIL_WARNINGS_AS_ERRORS}>:-Werror>)
endif()

if(WIN32)
  target_compile_definitions(sigil_compiler_flags INTERFACE
    WIN32_LEAN_AND_MEAN
    NOMINMAX)
endif()

if(SIGIL_SANITIZE)
  if(MSVC)
    if(NOT SIGIL_SANITIZE STREQUAL "address")
      message(FATAL_ERROR "MSVC supports only SIGIL_SANITIZE=address")
    endif()
    target_compile_options(sigil_compiler_flags INTERFACE /fsanitize=address)
    target_link_options(sigil_compiler_flags INTERFACE /INCREMENTAL:NO)
    string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  else()
    target_compile_options(sigil_compiler_flags INTERFACE
      -fsanitize=${SIGIL_SANITIZE}
      -fno-omit-frame-pointer
      -fno-sanitize-recover=all)
    target_link_options(sigil_compiler_flags INTERFACE -fsanitize=${SIGIL_SANITIZE})
  endif()
endif()

function(sigil_apply_target_defaults target)
  target_link_libraries(${target} PRIVATE sigil::compiler_flags)
  set_target_properties(${target} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)
endfunction()
