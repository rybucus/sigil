#pragma once

#include <sigil/platform.hxx>
#include <sigil/concepts.hxx>

#include <array>
#include <cstdint>
#include <immintrin.h>
#include <type_traits>

namespace sigil::bytes {

template<same_as<std::uint16_t> t>
SIGIL_FORCE_INLINE constexpr t concat( std::uint8_t v0, std::uint8_t v1 ) noexcept {
	return v0 | ( v1 << 8 );
}

template<same_as<std::uint32_t> t>
SIGIL_FORCE_INLINE constexpr t concat( std::uint8_t v0, std::uint8_t v1,
	std::uint8_t v2, std::uint8_t v3 ) noexcept {
	return v0 | ( v1 << 8 ) | ( v2 << 16 ) | ( v3 << 24 );
}

template<same_as<std::uint64_t> t>
SIGIL_FORCE_INLINE constexpr t concat( std::uint8_t v0, std::uint8_t v1,
	std::uint8_t v2, std::uint8_t v3, std::uint8_t v4, std::uint8_t v5,
	std::uint8_t v6, std::uint8_t v7 ) noexcept {
	return std::uint64_t( v0 ) | ( std::uint64_t( v1 ) << 8 )
		| ( std::uint64_t( v2 ) << 16 ) | ( std::uint64_t( v3 ) << 24 )
		| ( std::uint64_t( v4 ) << 32 ) | ( std::uint64_t( v5 ) << 40 )
		| ( std::uint64_t( v6 ) << 48 ) | ( std::uint64_t( v7 ) << 56 );
}

} // namespace sigil::bytes

namespace sigil::bits {

template<uint_t t>
SIGIL_FORCE_INLINE constexpr t clear_lowest_bit( t value ) noexcept {
	return value & ( value - 1 );
}

SIGIL_FORCE_INLINE constexpr std::uint32_t trailing_zero_count( std::uint32_t value ) noexcept {
#if SIGIL_MSVC
	if ( std::is_constant_evaluated( ) ) {
		std::uint32_t flag{ 1 };
		for ( std::uint8_t i{ }; i != sizeof( value ) * 8; ++i ) {
			if ( flag & value ) {
				return i;
			}
			flag <<= 1;
		}
		return sizeof( value ) * 8;
	} else {
		unsigned long result;
		_BitScanForward( &result, value );
		return static_cast< std::uint32_t >( result );
	}
#else
	return static_cast< std::uint32_t >( __builtin_ctz( value ) );
#endif
}

} // namespace sigil::bits

#if SIGIL_MSVC
#define SIGIL_DEFINE_AVX_BYTES( name, value )                                \
	static constexpr __m256i name{                                                 \
		.m256i_u8{ value, value, value, value, value, value, value, value,           \
			value, value, value, value, value, value, value, value,                    \
			value, value, value, value, value, value, value, value,                    \
			value, value, value, value, value, value, value, value }                   \
	}
#else
#define SIGIL_DEFINE_AVX_BYTES( name, value )                                \
	static constexpr __m256i name{ __builtin_bit_cast( __m256i,                    \
		std::array{ value, value, value, value, value, value, value, value,          \
			value, value, value, value, value, value, value, value,                    \
			value, value, value, value, value, value, value, value,                    \
			value, value, value, value, value, value, value, value } ) }
#endif
