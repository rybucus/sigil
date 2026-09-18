#pragma once

#include <sigil/platform.hxx>
#include <sigil/concepts.hxx>
#include <sigil/traits.hxx>
#include <sigil/bits.hxx>
#include <sigil/pattern.hxx>

#include <cstddef>
#include <cstdint>
#include <immintrin.h>
#include <optional>
#include <type_traits>

namespace sigil {

template<auto pattern_v, std::intptr_t offset_v>
struct scanner_t {
	using pattern_t = decltype( pattern_v );

	static constexpr std::intptr_t m_offset{ offset_v };
	static constexpr std::size_t m_num_elements{ pattern_t::m_num_elements };

	static constexpr std::optional<std::size_t> m_first_byte{ [ ] ( ) -> std::optional<std::size_t> {
		for ( std::size_t i{ }; i < pattern_t::m_elements.size( ); ++i ) {
			if ( pattern_t::m_elements[ i ].m_byte ) {
				return i;
			}
		}
		return { };
	}( ) };

	static constexpr std::optional<std::size_t> m_last_byte{ [ ] ( ) -> std::optional<std::size_t> {
		std::size_t result{ pattern_t::m_elements.size( ) };
		for ( std::size_t i{ }; i < pattern_t::m_elements.size( ); ++i ) {
			if ( pattern_t::m_elements[ i ].m_byte ) {
				result = i;
			}
		}
		if ( result == pattern_t::m_elements.size( ) ) {
			return { };
		}
		return result;
	}( ) };

	template<sized_as<1> uint8_t, callable<void( uint8_t* const )>... action_t>
		requires( !m_first_byte.has_value( ) )
	SIGIL_FORCE_INLINE static std::conditional_t<sizeof...( action_t ) == 0, uint8_t*, void>
	search_unbounded( uint8_t* begin, action_t&&... action ) noexcept {
		for ( ;; ) {
			if ( pattern_t::is_match( begin ) ) [[unlikely]] {
				( action( begin + m_offset ), ... );
				if constexpr ( sizeof...( action ) == 0 ) {
					return begin + m_offset;
				}
			}
			++begin;
		}
	}

	template<sized_as<1> uint8_t, callable<void( uint8_t* const )>... action_t>
		requires( m_first_byte.has_value( ) && m_last_byte.has_value( )
			&& *m_first_byte == *m_last_byte )
	SIGIL_FORCE_INLINE static std::conditional_t<sizeof...( action_t ) == 0, uint8_t*, void>
	search_unbounded( uint8_t* begin, action_t&&... action ) noexcept {
		static constexpr auto first_index{ compact<*m_first_byte> };
		SIGIL_DEFINE_AVX_BYTES( first, *pattern_t::m_elements[ first_index ].m_byte );

		for ( ;; begin += sizeof( __m256i ) ) {
			const __m256i block{ _mm256_loadu_si256(
				reinterpret_cast< const __m256i* >( begin + first_index ) ) };
			const __m256i eq{ _mm256_cmpeq_epi8( first, block ) };

			for ( std::uint32_t mask{ static_cast< std::uint32_t >( _mm256_movemask_epi8( eq ) ) };
				mask; mask = bits::clear_lowest_bit( mask ) ) [[unlikely]] {
				const auto bit_pos{ bits::trailing_zero_count( mask ) };
				if ( decltype( pattern_t::template delete_element<first_index>( ) )::is_match(
					begin + bit_pos ) ) [[unlikely]] {
					( action( begin + bit_pos + m_offset ), ... );
					if constexpr ( sizeof...( action ) == 0 ) {
						return begin + bit_pos + m_offset;
					}
				}
			}
		}
	}

	template<sized_as<1> uint8_t, callable<void( uint8_t* const )>... action_t>
		requires( m_first_byte.has_value( ) && m_last_byte.has_value( )
			&& *m_first_byte != *m_last_byte )
	SIGIL_FORCE_INLINE static std::conditional_t<sizeof...( action_t ) == 0, uint8_t*, void>
	search_unbounded( uint8_t* begin, action_t&&... action ) noexcept {
		static constexpr auto first_index{ compact<*m_first_byte> };
		static constexpr auto last_index{ compact<*m_last_byte> };
		SIGIL_DEFINE_AVX_BYTES( first, *pattern_t::m_elements[ first_index ].m_byte );
		SIGIL_DEFINE_AVX_BYTES( last, *pattern_t::m_elements[ last_index ].m_byte );

		for ( ;; begin += sizeof( __m256i ) ) {
			const __m256i block_first{ _mm256_loadu_si256(
				reinterpret_cast< const __m256i* >( begin + first_index ) ) };
			const __m256i block_last{ _mm256_loadu_si256(
				reinterpret_cast< const __m256i* >( begin + last_index ) ) };
			const __m256i eq_first{ _mm256_cmpeq_epi8( first, block_first ) };
			const __m256i eq_last{ _mm256_cmpeq_epi8( last, block_last ) };

			for ( std::uint32_t mask{ static_cast< std::uint32_t >( _mm256_movemask_epi8(
				_mm256_and_si256( eq_first, eq_last ) ) ) }; mask != 0;
				mask = bits::clear_lowest_bit( mask ) ) [[unlikely]] {
				const auto bit_pos{ bits::trailing_zero_count( mask ) };
				if ( decltype( pattern_t::template delete_element<first_index, last_index>( ) )::is_match(
					begin + bit_pos ) ) [[unlikely]] {
					( action( begin + bit_pos + m_offset ), ... );
					if constexpr ( sizeof...( action ) == 0 ) {
						return begin + bit_pos + m_offset;
					}
				}
			}
		}
	}

	template<sized_as<1> uint8_t, callable<void( uint8_t* const )>... action_t>
		requires( !m_first_byte.has_value( ) )
	SIGIL_FORCE_INLINE static std::conditional_t<sizeof...( action_t ) == 0, uint8_t*, void>
	search( uint8_t* begin, std::size_t size, action_t&&... action ) noexcept {
		if ( size >= m_num_elements ) [[likely]] {
			uint8_t* const end{ begin + size - m_num_elements };
			while ( begin <= end ) [[likely]] {
				if ( pattern_t::is_match( begin ) ) [[unlikely]] {
					( action( begin + m_offset ), ... );
					if constexpr ( sizeof...( action ) == 0 ) {
						return begin + m_offset;
					}
				}
				++begin;
			}
		}

		if constexpr ( sizeof...( action ) == 0 ) {
			return nullptr;
		}
	}

	template<sized_as<1> uint8_t, callable<void( uint8_t* const )>... action_t>
		requires( m_first_byte.has_value( ) && m_last_byte.has_value( )
			&& *m_first_byte == *m_last_byte )
	SIGIL_FORCE_INLINE static std::conditional_t<sizeof...( action_t ) == 0, uint8_t*, void>
	search( uint8_t* begin, std::size_t size, action_t&&... action ) noexcept {
		static constexpr auto first_index{ compact<*m_first_byte> };
		SIGIL_DEFINE_AVX_BYTES( first, *pattern_t::m_elements[ first_index ].m_byte );

		if ( size >= m_num_elements ) [[likely]] {
			uint8_t* const end{ begin + size - m_num_elements };
			if ( size >= m_num_elements + sizeof( __m256i ) ) [[likely]] {
				uint8_t* const simd_end{ begin + size - m_num_elements - sizeof( __m256i ) };
				for ( ; begin <= simd_end; begin += sizeof( __m256i ) ) {
					const __m256i block{ _mm256_loadu_si256(
						reinterpret_cast< const __m256i* >( begin + first_index ) ) };
					const __m256i eq{ _mm256_cmpeq_epi8( first, block ) };

					for ( std::uint32_t mask{ static_cast< std::uint32_t >(
						_mm256_movemask_epi8( eq ) ) }; mask;
						mask = bits::clear_lowest_bit( mask ) ) [[unlikely]] {
						const auto bit_pos{ bits::trailing_zero_count( mask ) };
						if ( decltype( pattern_t::template delete_element<first_index>( ) )::is_match(
							begin + bit_pos ) ) [[unlikely]] {
							( action( begin + bit_pos + m_offset ), ... );
							if constexpr ( sizeof...( action ) == 0 ) {
								return begin + bit_pos + m_offset;
							}
						}
					}
				}
			}

			[[unlikely]];
			for ( ; begin <= end; ++begin ) {
				if ( pattern_t::is_match( begin ) ) [[unlikely]] {
					( action( begin + m_offset ), ... );
					if constexpr ( sizeof...( action ) == 0 ) {
						return begin + m_offset;
					}
				}
			}
		}

		if constexpr ( sizeof...( action ) == 0 ) {
			return nullptr;
		}
	}

	template<sized_as<1> uint8_t, callable<void( uint8_t* const )>... action_t>
		requires( m_first_byte.has_value( ) && m_last_byte.has_value( )
			&& *m_first_byte != *m_last_byte )
	SIGIL_FORCE_INLINE static std::conditional_t<sizeof...( action_t ) == 0, uint8_t*, void>
	search( uint8_t* begin, std::size_t size, action_t&&... action ) noexcept {
		static constexpr auto first_index{ compact<*m_first_byte> };
		static constexpr auto last_index{ compact<*m_last_byte> };
		SIGIL_DEFINE_AVX_BYTES( first, *pattern_t::m_elements[ first_index ].m_byte );
		SIGIL_DEFINE_AVX_BYTES( last, *pattern_t::m_elements[ last_index ].m_byte );

		if ( size >= m_num_elements ) [[likely]] {
			uint8_t* const end{ begin + size - m_num_elements };
			if ( size >= sizeof( __m256i ) + last_index ) [[likely]] {
				uint8_t* const simd_end{ begin + size - sizeof( __m256i ) - last_index };
				for ( ; begin <= simd_end; begin += sizeof( __m256i ) ) [[likely]] {
					const __m256i block_first{ _mm256_loadu_si256(
						reinterpret_cast< const __m256i* >( begin + first_index ) ) };
					const __m256i block_last{ _mm256_loadu_si256(
						reinterpret_cast< const __m256i* >( begin + last_index ) ) };
					const __m256i eq_first{ _mm256_cmpeq_epi8( first, block_first ) };
					const __m256i eq_last{ _mm256_cmpeq_epi8( last, block_last ) };

					for ( std::uint32_t mask{ static_cast< std::uint32_t >( _mm256_movemask_epi8(
						_mm256_and_si256( eq_first, eq_last ) ) ) }; mask != 0;
						mask = bits::clear_lowest_bit( mask ) ) [[unlikely]] {
						const auto bit_pos{ bits::trailing_zero_count( mask ) };
						if ( decltype( pattern_t::template delete_element<first_index, last_index>( ) )::is_match(
							begin + bit_pos ) ) [[unlikely]] {
							( action( begin + bit_pos + m_offset ), ... );
							if constexpr ( sizeof...( action ) == 0 ) {
								return begin + bit_pos + m_offset;
							}
						}
					}
				}
			}

			[[unlikely]];
			for ( ; begin <= end; ++begin ) {
				if ( pattern_t::is_match( begin ) ) [[unlikely]] {
					( action( begin + m_offset ), ... );
					if constexpr ( sizeof...( action ) == 0 ) {
						return begin + m_offset;
					}
				}
			}
		}

		if constexpr ( sizeof...( action ) == 0 ) {
			return nullptr;
		}
	}

	template<sized_as<1> uint8_t, callable<void( uint8_t* const )>... action_t>
	SIGIL_FORCE_INLINE static std::conditional_t<sizeof...( action_t ) == 0, uint8_t*, void>
	search( uint8_t* begin, uint8_t* end, action_t&&... action ) noexcept {
		return search( begin, static_cast< std::size_t >( end - begin ),
			SIGIL_FWD( action )... );
	}

	template<linear_container_any_of<char, std::uint8_t> container_t,
		callable<void( element_of_t<container_t>* )>... action_t>
	SIGIL_FORCE_INLINE static auto search( container_t&& bytes,
		action_t&&... action ) noexcept {
		return search( bytes.data( ), bytes.size( ), SIGIL_FWD( action )... );
	}
};

template<fixed_string_t str, std::intptr_t offset = 0>
constexpr scanner_t<make_byte_pattern<str>( ), offset> byte_scanner{ };

} // namespace sigil
