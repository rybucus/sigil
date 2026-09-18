#pragma once

#include <sigil/platform.hxx>
#include <sigil/traits.hxx>

#include <cstddef>
#include <cstdint>

namespace sigil {

template<typename, std::size_t>
struct const_vector_t;

template<typename t, std::size_t size_v>
	requires( size_v > 0 )
struct const_vector_t<t, size_v> {
public:
	using index_t = compact_t<size_v>;

public:
	constexpr const_vector_t( auto&&... args ) noexcept
		requires( sizeof...( args ) <= size_v )
		: m_data{ SIGIL_FWD( args )... }, m_size{ sizeof...( args ) } { }

public:
	constexpr index_t size( ) const noexcept { return m_size; }

	template<typename self_t>
	constexpr like_t<self_t, t> front( this self_t&& self ) noexcept { return SIGIL_FWD( self ).m_data[ 0 ]; }

	template<typename self_t>
	constexpr like_t<self_t, t> back( this self_t&& self ) noexcept { return SIGIL_FWD( self ).m_data[ self.m_size - 1 ]; }

	template<typename self_t>
	constexpr like_ptr_t<self_t, t> data( this self_t&& self ) noexcept { return self.m_data; }

	template<typename self_t>
	constexpr like_ptr_t<self_t, t> begin( this self_t&& self ) noexcept { return self.m_data; }

	template<typename self_t>
	constexpr like_ptr_t<self_t, t> end( this self_t&& self ) noexcept { return self.begin( ) + self.m_size; }

public:
	constexpr void clear( ) noexcept { m_size = { }; }

	constexpr void push_back( t entry ) noexcept { m_data[ m_size++ ] = entry; }

public:
	template<typename self_t>
	constexpr like_t<self_t, t> operator[]( this self_t&& self, index_t index ) noexcept {
		return static_cast< like_t<self_t, t> >( SIGIL_FWD( self ).m_data[ index ] );
	}

public:
	static constexpr index_t capacity( ) noexcept { return size_v; }

public:
	t m_data[ size_v ];
	index_t m_size;

};

template<typename t>
struct const_vector_t<t, 0> {
public:
	constexpr const_vector_t( ) noexcept = default;

public:
	constexpr const t* data( ) const noexcept { return nullptr; }

	constexpr const t* begin( ) const noexcept { return nullptr; }

	constexpr const t* end( ) const noexcept { return nullptr; }

public:
	static constexpr std::uint8_t size( ) noexcept { return 0; }

	static constexpr std::uint8_t capacity( ) noexcept { return 0; }

};

} // namespace sigil
