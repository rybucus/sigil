#pragma once

#include <sigil/platform.hxx>
#include <sigil/concepts.hxx>

#include <algorithm>
#include <type_traits>
#include <utility>

namespace sigil::algorithm {

template<typename arg_t, typename... args_t>
	requires( all_same_v<arg_t, args_t...> && std::is_trivially_copyable_v<arg_t>
		&& sizeof( arg_t ) <= 2 * sizeof( void* ) )
SIGIL_FORCE_INLINE constexpr arg_t max( arg_t arg, const args_t... args ) noexcept {
	( ( arg = ( args > arg ) ? args : arg ), ... );
	return arg;
}

template<has_data_size container_t>
SIGIL_FORCE_INLINE constexpr void bubble_sort( container_t&& container,
	callable<bool( const element_of_t<container_t>,
		const element_of_t<container_t> )> auto&& compare ) noexcept {
	using index_t = std::remove_cvref_t<decltype( container.size( ) )>;

	const index_t size{ container.size( ) };
	if ( size ) {
		for ( index_t i{ }; i != size - 1; ++i ) {
			for ( index_t j{ }; j < size - i - 1; ++j ) {
				if ( !SIGIL_FWD( compare )( container[ j ], container[ j + 1 ] ) ) {
					std::swap( container[ j ], container[ j + 1 ] );
				}
			}
		}
	}
}

template<has_begin_end container_t>
SIGIL_FORCE_INLINE constexpr void stable_sort( container_t&& container,
	callable<bool( const element_of_t<container_t>,
		const element_of_t<container_t> )> auto&& compare ) noexcept {
	if ( std::is_constant_evaluated( ) ) {
		bubble_sort( SIGIL_FWD( container ), SIGIL_FWD( compare ) );
	} else {
		std::stable_sort( container.begin( ), container.end( ), SIGIL_FWD( compare ) );
	}
}

} // namespace sigil::algorithm
