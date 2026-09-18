#pragma once

#include <sigil/platform.hxx>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace sigil {

#if SIGIL_CLANG || SIGIL_GCC
template<typename t, typename u>
concept same_as = __is_same( t, u );

template<typename t, typename... args_t>
constexpr bool all_same_v{ ( __is_same( t, args_t ) && ... ) };

template<typename t, typename... args_t>
concept any_of = ( __is_same( t, args_t ) || ... );
#else
template<typename t, typename u>
concept same_as = std::is_same_v<t, u>;

template<typename t, typename... args_t>
constexpr bool all_same_v{ ( std::is_same_v<t, args_t> && ... ) };

template<typename t, typename... args_t>
concept any_of = ( std::is_same_v<t, args_t> || ... );
#endif

template<typename, typename sig_t>
struct callable_impl_t;

template<typename t, typename ret_t, typename... args_t>
struct callable_impl_t<t, ret_t( args_t... )> {
	static constexpr bool value{ requires( t fn, args_t... args ) {
		{ SIGIL_FWD( fn )( SIGIL_FWD( args )... ) } -> same_as<ret_t>;
	} };
};

template<typename t, typename sig_t>
concept callable = callable_impl_t<t, sig_t>::value;

template<typename t, std::size_t size_v>
concept sized_as = sizeof( t ) == size_v;

template<typename t>
concept has_begin_end = requires( const t container ) {
	container.begin( ) != container.end( );
};

template<typename t>
concept has_data_size = requires( const t container ) {
	container.data( ) + container.size( );
};

template<typename t>
concept c_array = std::is_array<typename std::remove_cvref<t>::type>::value;

template<typename t>
concept has_dtor = !std::is_trivially_destructible_v<t>;

template<typename t>
concept not_cvref = same_as<t, std::remove_cvref_t<t>>;

template<typename t, typename u>
concept assignable = requires( u& dst, const t src ) {
	dst = src;
};

template<typename t>
concept uint_t = std::is_integral_v<std::remove_cvref_t<t>>
	&& std::is_unsigned_v<std::remove_cvref_t<t>>;

template<typename container_t, typename t>
concept linear_container_of = requires {
	*( std::declval<container_t>( ).data( ) + std::declval<container_t>( ).size( ) );
} && same_as<std::remove_cvref_t<decltype( *( std::declval<container_t>( ).data( )
	+ std::declval<container_t>( ).size( ) ) )>, std::remove_cvref_t<t>>;

template<typename container_t, typename... args_t>
concept linear_container_any_of = ( linear_container_of<container_t, args_t> || ... );

namespace detail {

template<typename container_t>
struct element_of_impl_t {
	using type = std::remove_cvref_t<
		decltype( *std::declval<const std::remove_cvref_t<container_t>>( ).data( ) )
	>;
};

template<c_array container_t>
struct element_of_impl_t<container_t> {
	using type = std::remove_cvref_t<decltype( *std::declval<container_t>( ) )>;
};

} // namespace detail

template<typename container_t>
using element_of_t = typename detail::element_of_impl_t<container_t>::type;

} // namespace sigil
