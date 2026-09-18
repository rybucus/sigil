#pragma once

#include <type_traits>

#if defined( __clang__ )
#define SIGIL_CLANG 1
#else
#define SIGIL_CLANG 0
#endif

#if defined( __GNUC__ ) && !defined( __clang__ )
#define SIGIL_GCC 1
#else
#define SIGIL_GCC 0
#endif

#if defined( _MSC_VER ) && !defined( __clang__ )
#define SIGIL_MSVC 1
#else
#define SIGIL_MSVC 0
#endif

#if defined( _MSC_VER ) && defined( __clang__ )
#define SIGIL_CLANG_CL 1
#else
#define SIGIL_CLANG_CL 0
#endif

static_assert( SIGIL_CLANG + SIGIL_GCC + SIGIL_MSVC == 1 );

#if SIGIL_MSVC || SIGIL_CLANG_CL
#if _WIN32 || _WIN64
#if _WIN64
#define SIGIL_BITNESS 64
#else
#define SIGIL_BITNESS 32
#endif
#endif
#else
#if __x86_64__ || __ppc64__
#define SIGIL_BITNESS 64
#else
#define SIGIL_BITNESS 32
#endif
#endif

static_assert( sizeof( void* ) * 8 == SIGIL_BITNESS );

#if SIGIL_CLANG || SIGIL_GCC
#define SIGIL_UNREACHABLE( ) __builtin_unreachable( )
#define SIGIL_FORCE_INLINE __attribute__( ( always_inline ) ) inline
#define SIGIL_LAMBDA_INLINE __attribute__( ( always_inline ) )
#else
#define SIGIL_UNREACHABLE( ) __assume( false )
#define SIGIL_FORCE_INLINE __forceinline
#define SIGIL_LAMBDA_INLINE [[msvc::forceinline]]
#endif

#define SIGIL_FWD( ... ) static_cast< decltype( __VA_ARGS__ )&& >( __VA_ARGS__ )

#define SIGIL_MOV( ... ) \
	static_cast< typename std::remove_reference<decltype( __VA_ARGS__ )>::type&& >( __VA_ARGS__ )
