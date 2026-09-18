#pragma once

#include <sigil/platform.hxx>
#include <sigil/concepts.hxx>
#include <sigil/traits.hxx>

#include <memory>

namespace sigil {

template<not_cvref t>
struct optional_t {
public:
	SIGIL_FORCE_INLINE constexpr optional_t( ) noexcept
		: m_has_value{ } { }

	SIGIL_FORCE_INLINE constexpr optional_t( auto&&... args ) noexcept
		requires( requires { t{ SIGIL_FWD( args )... }; } )
		: m_value{ SIGIL_FWD( args )... }, m_has_value{ true } { }

	SIGIL_FORCE_INLINE constexpr optional_t( const optional_t& rhs ) noexcept
		: m_has_value{ rhs.m_has_value } {
		if ( m_has_value ) {
			std::construct_at( std::addressof( m_value ), rhs.m_value );
		}
	}

	SIGIL_FORCE_INLINE constexpr optional_t( optional_t&& rhs ) noexcept
		: m_has_value{ rhs.m_has_value } {
		if ( m_has_value ) {
			std::construct_at( std::addressof( m_value ), SIGIL_MOV( rhs.m_value ) );
			rhs.m_has_value = false;
		}
	}

	SIGIL_FORCE_INLINE constexpr ~optional_t( ) noexcept {
		if constexpr ( has_dtor<t> ) {
			if ( m_has_value ) {
				m_value.~t( );
			}
		}
	}

public:
	SIGIL_FORCE_INLINE constexpr bool has_value( ) const noexcept { return m_has_value; }

	template<typename self_t>
	SIGIL_FORCE_INLINE constexpr like_t<self_t, t> value( this self_t&& self ) noexcept { return SIGIL_FWD( self ).m_value; }

public:
	SIGIL_FORCE_INLINE constexpr void emplace( auto&&... args ) noexcept
		requires( requires { t{ SIGIL_FWD( args )... }; } ) {
		std::construct_at( this, SIGIL_FWD( args )... );
		m_has_value = true;
	}

	SIGIL_FORCE_INLINE constexpr void reset( ) noexcept {
		if ( m_has_value ) {
			if constexpr ( has_dtor<t> ) {
				m_value.~t( );
			}
			m_has_value = false;
		}
	}

public:
	SIGIL_FORCE_INLINE constexpr optional_t& operator=( const optional_t& rhs ) noexcept {
		( *this ).~optional_t( );
		std::construct_at( this, rhs );
		return *this;
	}

	SIGIL_FORCE_INLINE constexpr optional_t& operator=( optional_t&& rhs ) noexcept {
		( *this ).~optional_t( );
		std::construct_at( this, SIGIL_MOV( rhs ) );
		return *this;
	}

	SIGIL_FORCE_INLINE constexpr optional_t& operator=( auto&& args ) noexcept
		requires( requires { t{ SIGIL_FWD( args ) }; } ) {
		( *this ).~optional_t( );
		std::construct_at( this, SIGIL_FWD( args ) );
		return *this;
	}

	SIGIL_FORCE_INLINE constexpr explicit operator bool( ) const noexcept { return m_has_value; }

	template<typename self_t>
	SIGIL_FORCE_INLINE constexpr like_t<self_t, t> operator*( this self_t&& self ) noexcept { return SIGIL_FWD( self ).m_value; }

	template<typename self_t>
	SIGIL_FORCE_INLINE constexpr like_ptr_t<self_t, t> operator->( this self_t&& self ) noexcept { return std::addressof( self.m_value ); }

public:
#if SIGIL_GCC
	t m_value{ };
#else
	union {
		t m_value;
	};
#endif
	bool m_has_value;

};

template<typename t>
SIGIL_FORCE_INLINE constexpr bool operator==( const optional_t<t>& lhs,
	const optional_t<t>& rhs ) noexcept {
	if ( lhs.has_value( ) ) {
		return rhs.has_value( ) && *lhs == *rhs;
	}
	return !rhs.has_value( );
}

template<typename t>
SIGIL_FORCE_INLINE constexpr bool operator!=( const optional_t<t>& lhs,
	const optional_t<t>& rhs ) noexcept {
	return !( lhs == rhs );
}

template<typename t>
SIGIL_FORCE_INLINE constexpr bool operator==( const optional_t<t>& lhs,
	const t& rhs ) noexcept {
	return lhs && ( *lhs == rhs );
}

template<typename t>
SIGIL_FORCE_INLINE constexpr bool operator==( const t& lhs,
	const optional_t<t>& rhs ) noexcept {
	return rhs && ( lhs == *rhs );
}

template<typename t>
SIGIL_FORCE_INLINE constexpr bool operator!=( const optional_t<t>& lhs,
	const t& rhs ) noexcept {
	return !lhs || ( *lhs != rhs );
}

template<typename t>
SIGIL_FORCE_INLINE constexpr bool operator!=( const t& lhs,
	const optional_t<t>& rhs ) noexcept {
	return !rhs || ( lhs != *rhs );
}

} // namespace sigil
