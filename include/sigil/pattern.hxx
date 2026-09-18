#pragma once

#include <sigil/platform.hxx>
#include <sigil/concepts.hxx>
#include <sigil/traits.hxx>
#include <sigil/algorithm.hxx>
#include <sigil/const_vector.hxx>
#include <sigil/fixed_string.hxx>
#include <sigil/optional.hxx>
#include <sigil/bits.hxx>

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace sigil {

struct nibbles_t {
	std::uint8_t low : 4;
	std::uint8_t high : 4;
};

struct comparison_entry_nibble_t {
	std::size_t m_offset;
	optional_t<std::uint8_t> m_low;
	optional_t<std::uint8_t> m_high;
};

struct comparison_entry_bytes_t {
	std::size_t m_offset{ };
	std::uint8_t m_size{ };
};

struct element_t {
public:
	constexpr element_t( optional_t<std::uint8_t> low,
		optional_t<std::uint8_t> high ) noexcept
		: m_low{ low }, m_high{ high },
		m_byte{ ( low && high )
			? static_cast< std::uint8_t >( *low | ( *high << 4 ) )
			: optional_t<std::uint8_t>{ } } { }

	constexpr element_t( std::uint8_t byte ) noexcept
		: m_low{ static_cast< std::uint8_t >( byte & 0x0f ) },
		m_high{ static_cast< std::uint8_t >( static_cast< std::uint8_t >( byte & 0xf0 ) >> 4 ) },
		m_byte{ ( m_low && m_high )
			? static_cast< std::uint8_t >( *m_low | ( *m_high << 4 ) )
			: optional_t<std::uint8_t>{ } } { }

	constexpr element_t( ) noexcept = default;

public:
	constexpr bool operator==( const element_t& rhs ) const noexcept { return m_low == rhs.m_low && m_high == rhs.m_high; }

public:
	optional_t<std::uint8_t> m_low;
	optional_t<std::uint8_t> m_high;
	optional_t<std::uint8_t> m_byte;

};

template<auto elements_v>
struct sequence_t {
	static constexpr std::size_t m_num_elements{ elements_v.size( ) };
	static constexpr std::array<element_t, m_num_elements> m_elements{ elements_v };

	static constexpr const_vector_t<comparison_entry_bytes_t, m_num_elements> m_bytes_entries{ [ ] {
		const_vector_t<comparison_entry_bytes_t, m_num_elements> result;
		std::size_t start{ };

		for ( ;; ) {
			while ( start < m_num_elements && !m_elements[ start ].m_byte ) {
				++start;
			}
			if ( start >= m_num_elements ) {
				break;
			}

			std::size_t next_non_byte{ start + 1 };
			while ( next_non_byte < m_num_elements && m_elements[ next_non_byte ].m_byte ) {
				++next_non_byte;
			}

			const std::size_t distance{ next_non_byte - start };
			const std::uint8_t compare_length{
				( distance >= 8 && sizeof( void* ) >= 8 ) ? std::uint8_t( 8 )
				: ( distance >= 4 ? std::uint8_t( 4 )
					: ( distance >= 2 ? std::uint8_t( 2 ) : std::uint8_t( 1 ) ) )
			};

			result.push_back( { start, compare_length } );
			start += compare_length;
		}

		algorithm::stable_sort( result, [ ] ( const comparison_entry_bytes_t& lhs,
			const comparison_entry_bytes_t& rhs ) {
			return lhs.m_size > rhs.m_size;
		} );

		return result;
	}( ) };

	static constexpr const_vector_t<comparison_entry_nibble_t, m_num_elements> m_nibble_entries{ [ ] {
		const_vector_t<comparison_entry_nibble_t, m_num_elements> result;
		std::size_t offset{ };

		for ( element_t element : m_elements ) {
			if ( element.m_low.has_value( ) ^ element.m_high.has_value( ) ) {
				result.push_back( { offset, element.m_low, element.m_high } );
			}
			++offset;
		}

		return result;
	}( ) };

	template<auto index, sized_as<1> uint8_t>
	SIGIL_FORCE_INLINE static constexpr bool compare_bytes_entry( uint8_t* ptr ) noexcept {
		static constexpr comparison_entry_bytes_t entry{ m_bytes_entries[ index ] };
		static constexpr std::size_t offset{ entry.m_offset };
		static constexpr std::uint8_t size{ entry.m_size };
		static constexpr auto bytes{ [ ] ( ) -> unsigned_bits_t<size * 8> {
			if constexpr ( size == 1 ) {
				return *m_elements[ offset ].m_byte;
			} else if constexpr ( size == 2 ) {
				return bytes::concat<std::uint16_t>( *m_elements[ offset ].m_byte,
					*m_elements[ offset + 1 ].m_byte );
			} else if constexpr ( size == 4 ) {
				return bytes::concat<std::uint32_t>( *m_elements[ offset ].m_byte,
					*m_elements[ offset + 1 ].m_byte, *m_elements[ offset + 2 ].m_byte,
					*m_elements[ offset + 3 ].m_byte );
			} else if constexpr ( size == 8 ) {
				return bytes::concat<std::uint64_t>( *m_elements[ offset ].m_byte,
					*m_elements[ offset + 1 ].m_byte, *m_elements[ offset + 2 ].m_byte,
					*m_elements[ offset + 3 ].m_byte, *m_elements[ offset + 4 ].m_byte,
					*m_elements[ offset + 5 ].m_byte, *m_elements[ offset + 6 ].m_byte,
					*m_elements[ offset + 7 ].m_byte );
			}
		}( ) };

		return *reinterpret_cast< const decltype( bytes )* >( ptr + offset ) == bytes;
	}

	template<auto index, sized_as<1> uint8_t>
	SIGIL_FORCE_INLINE static constexpr bool compare_nibble_entry( uint8_t* ptr ) noexcept {
		static constexpr comparison_entry_nibble_t entry{ m_nibble_entries[ index ] };
		static constexpr std::size_t offset{ entry.m_offset };
		static constexpr optional_t<std::uint8_t> low{ entry.m_low };
		static constexpr optional_t<std::uint8_t> high{ entry.m_high };

		if constexpr ( low ) {
			return reinterpret_cast< const nibbles_t* >( ptr + offset )->low == constant<*low>;
		} else if constexpr ( high ) {
			return reinterpret_cast< const nibbles_t* >( ptr + offset )->high == constant<*high>;
		}
	}

	template<sized_as<1> uint8_t, auto... bytes_index, auto... nibble_index>
	SIGIL_FORCE_INLINE static constexpr bool is_match_impl( uint8_t* ptr,
		std::index_sequence<bytes_index...>, std::index_sequence<nibble_index...> ) noexcept {
		return ( compare_bytes_entry<bytes_index>( ptr ) && ... )
			&& ( compare_nibble_entry<nibble_index>( ptr ) && ... );
	}

	template<sized_as<1> uint8_t>
	SIGIL_FORCE_INLINE static constexpr bool is_match( uint8_t* ptr ) noexcept {
		return is_match_impl( ptr, std::make_index_sequence<m_bytes_entries.size( )>{ },
			std::make_index_sequence<m_nibble_entries.size( )>{ } );
	}

	template<std::size_t... indexes>
	static consteval auto delete_element( ) noexcept {
		return sequence_t< [ ] <std::size_t... i>( std::index_sequence<i...> ) {
			std::array<element_t, m_num_elements> result{ m_elements };
			( ( result[ i ] = element_t{ } ), ... );
			return result;
		}( std::index_sequence<indexes...>{ } ) >{ };
	}
};

template<fixed_string_t str>
constexpr const_vector_t<element_t, str.size( )> parse_pattern_string( ) noexcept {
	constexpr auto is_hex_digit{ [ ] ( char c ) {
		return ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' )
			|| ( c >= 'A' && c <= 'F' );
	} };

	constexpr auto char_to_hex_digit{ [ ] ( char c ) -> std::uint8_t {
		if ( c >= '0' && c <= '9' ) {
			return c - '0';
		}
		if ( c >= 'a' && c <= 'f' ) {
			return c - 'a' + 10;
		}
		if ( c >= 'A' && c <= 'F' ) {
			return c - 'A' + 10;
		}
		SIGIL_UNREACHABLE( );
	} };

	constexpr std::size_t size{ str.size( ) };
	const char* begin{ str.data( ) };
	const char* end{ begin + size };
	const_vector_t<element_t, size> result;

	if constexpr ( size ) {
		bool expect_space{ };
		for ( const char* ch{ begin }; ch != end; ) {
			const char current{ *ch };
			if ( current == ' ' ) {
				if ( !expect_space ) {
					SIGIL_UNREACHABLE( );
				}
				++ch;
				expect_space = false;
				continue;
			}

			if ( expect_space ) {
				SIGIL_UNREACHABLE( );
			}

			if ( current == '?' ) {
				if ( ch + 1 == end ) {
					ch += 1;
					result.push_back( { } );
				} else {
					const char next{ ch[ 1 ] };
					if ( next == ' ' ) {
						ch += 1;
						result.push_back( { } );
					} else if ( next == '?' ) {
						ch += 2;
						result.push_back( { } );
					} else if ( is_hex_digit( next ) ) {
						result.push_back( element_t{ char_to_hex_digit( next ), { } } );
						ch += 2;
					} else {
						SIGIL_UNREACHABLE( );
					}
				}
			} else {
				if ( !is_hex_digit( current ) ) {
					SIGIL_UNREACHABLE( );
				}

				if ( ch + 1 == end ) {
					result.push_back( char_to_hex_digit( current ) );
					ch += 1;
				} else {
					const char next{ ch[ 1 ] };
					if ( next == ' ' ) {
						result.push_back( char_to_hex_digit( current ) );
						ch += 1;
					} else if ( next == '?' ) {
						result.push_back( element_t{ { }, char_to_hex_digit( current ) } );
						ch += 2;
					} else if ( is_hex_digit( next ) ) {
						result.push_back( static_cast< std::uint8_t >(
							char_to_hex_digit( current ) * 0x10 + char_to_hex_digit( next ) ) );
						ch += 2;
					} else {
						SIGIL_UNREACHABLE( );
					}
				}
			}

			expect_space = true;
		}
	}

	return result;
}

template<fixed_string_t str>
constexpr auto make_byte_pattern( ) noexcept {
	constexpr const_vector_t<element_t, str.size( )> elements{ parse_pattern_string<str>( ) };
	return [ = ] <auto... index>( std::index_sequence<index...> ) {
		return sequence_t< std::array<element_t, elements.size( )>{ elements[ index ]... } >{ };
	}( std::make_index_sequence<elements.size( )>{ } );
}

} // namespace sigil
