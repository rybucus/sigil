#pragma once

#include <sigil/platform.hxx>
#include <sigil/concepts.hxx>
#include <sigil/traits.hxx>

#include <cstddef>
#include <cstdint>
#include <utility>

namespace sigil {

template<typename char_t, std::size_t size_v>
struct fixed_string_t {
public:
	using char_type = char_t;

private:
	template<std::size_t... index>
	SIGIL_FORCE_INLINE constexpr fixed_string_t( const char_t( &str )[ size_v + 1 ],
		std::index_sequence<index...> ) noexcept
		: m_data{ str[ index ]... } { }

public:
	SIGIL_FORCE_INLINE constexpr fixed_string_t( const char_t( &str )[ size_v + 1 ] ) noexcept
		: fixed_string_t{ str, std::make_index_sequence<size_v>{ } } { }

	SIGIL_FORCE_INLINE constexpr fixed_string_t( auto... chars ) noexcept
		: m_data{ chars... } { }

public:
	SIGIL_FORCE_INLINE constexpr const char_t* begin( ) const noexcept { return m_data; }

	SIGIL_FORCE_INLINE constexpr const char_t* end( ) const noexcept { return m_data + size( ); }

	SIGIL_FORCE_INLINE constexpr const char_t* data( ) const noexcept { return begin( ); }

public:
	template<typename self_t>
	SIGIL_FORCE_INLINE constexpr like_t<self_t, char_t> operator[]( this self_t&& self,
		std::size_t index ) noexcept {
		return static_cast< like_t<self_t, char_t> >( SIGIL_FWD( self ).m_data[ index ] );
	}

	SIGIL_FORCE_INLINE constexpr bool operator==(
		const fixed_string_t<char_t, size_v>& ) const noexcept = default;

	SIGIL_FORCE_INLINE constexpr bool operator!=(
		const fixed_string_t<char_t, size_v>& ) const noexcept = default;

public:
	SIGIL_FORCE_INLINE static constexpr compact_t<size_v> size( ) noexcept { return size_v; }

public:
	char_t m_data[ size_v ];

};

template<typename char_t>
struct fixed_string_t<char_t, 0> {
public:
	using char_type = char_t;

public:
	SIGIL_FORCE_INLINE constexpr fixed_string_t( const char_t( & )[ 1 ] ) noexcept { }
	SIGIL_FORCE_INLINE constexpr fixed_string_t( ) noexcept = default;

public:
	SIGIL_FORCE_INLINE constexpr const char_t* begin( ) const noexcept { return nullptr; }

	SIGIL_FORCE_INLINE constexpr const char_t* end( ) const noexcept { return nullptr; }

	SIGIL_FORCE_INLINE constexpr const char_t* data( ) const noexcept { return nullptr; }

public:
	template<typename self_t>
	SIGIL_FORCE_INLINE constexpr like_t<self_t, char_t> operator[]( this self_t&&,
		std::size_t ) noexcept;

	SIGIL_FORCE_INLINE constexpr bool operator==( const fixed_string_t<char_t, 0>& ) const noexcept { return true; }

	SIGIL_FORCE_INLINE constexpr bool operator!=( const fixed_string_t<char_t, 0>& ) const noexcept { return false; }

public:
	SIGIL_FORCE_INLINE static constexpr std::uint8_t size( ) noexcept { return 0; }

};

template<typename char_t, std::size_t size_v>
fixed_string_t( const char_t( & )[ size_v ] ) -> fixed_string_t<char_t, size_v - 1>;

template<typename char_t, same_as<char_t>... chars_t>
fixed_string_t( char_t, chars_t... ) -> fixed_string_t<char_t, sizeof...( chars_t ) + 1>;

} // namespace sigil
