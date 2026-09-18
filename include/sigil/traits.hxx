#pragma once

#include <sigil/platform.hxx>
#include <sigil/algorithm.hxx>

#include <cstdint>
#include <type_traits>

namespace sigil {

namespace detail {

template<std::uint64_t max_value>
struct compact_impl_t;

template<std::uint64_t max_value>
	requires( max_value <= std::uint8_t( -1 ) )
struct compact_impl_t<max_value> {
	using type = std::uint8_t;
};

template<std::uint64_t max_value>
	requires( max_value > std::uint8_t( -1 ) && max_value <= std::uint16_t( -1 ) )
struct compact_impl_t<max_value> {
	using type = std::uint16_t;
};

template<std::uint64_t max_value>
	requires( max_value > std::uint16_t( -1 ) && max_value <= std::uint32_t( -1 ) )
struct compact_impl_t<max_value> {
	using type = std::uint32_t;
};

template<std::uint64_t max_value>
	requires( max_value > std::uint32_t( -1 ) )
struct compact_impl_t<max_value> {
	using type = std::uint64_t;
};

} // namespace detail

template<std::uint64_t... max_value>
using compact_t = typename detail::compact_impl_t<algorithm::max( max_value... )>::type;

template<std::uint64_t value>
static constexpr compact_t<value> compact{ static_cast< compact_t<value> >( value ) };

template<auto value>
constexpr auto constant{ value };

template<typename...>
struct like_impl_t;

template<typename from_t, typename to_t>
struct like_impl_t<from_t&, to_t> {
	using type = to_t&;
};

template<typename from_t, typename to_t>
struct like_impl_t<const from_t&, to_t> {
	using type = const to_t&;
};

template<typename from_t, typename to_t>
struct like_impl_t<volatile from_t&, to_t> {
	using type = volatile to_t&;
};

template<typename from_t, typename to_t>
struct like_impl_t<const volatile from_t&, to_t> {
	using type = const volatile to_t&;
};

template<typename from_t, typename to_t>
struct like_impl_t<from_t&&, to_t> {
	using type = to_t&&;
};

template<typename from_t, typename to_t>
struct like_impl_t<const from_t&&, to_t> {
	using type = const to_t&&;
};

template<typename from_t, typename to_t>
struct like_impl_t<volatile from_t&&, to_t> {
	using type = volatile to_t&&;
};

template<typename from_t, typename to_t>
struct like_impl_t<const volatile from_t&&, to_t> {
	using type = const volatile to_t&&;
};

template<typename from_t, typename to_t>
struct like_impl_t<from_t, to_t> {
	using type = to_t&&;
};

template<typename from_t, typename to_t>
struct like_impl_t<const from_t, to_t> {
	using type = const to_t&&;
};

template<typename from_t, typename to_t>
struct like_impl_t<volatile from_t, to_t> {
	using type = volatile to_t&&;
};

template<typename from_t, typename to_t>
struct like_impl_t<const volatile from_t, to_t> {
	using type = const volatile to_t&&;
};

template<typename from_t, typename to_t>
using like_t = typename like_impl_t<from_t, std::remove_cvref_t<to_t>>::type;

template<typename from_t, typename to_t>
using like_ptr_t = std::remove_reference_t<like_t<from_t, to_t>>*;

template<std::uint8_t bits_v>
struct unsigned_bits_impl_t { };

template<>
struct unsigned_bits_impl_t<8> {
	using type = std::uint8_t;
};

template<>
struct unsigned_bits_impl_t<16> {
	using type = std::uint16_t;
};

template<>
struct unsigned_bits_impl_t<32> {
	using type = std::uint32_t;
};

template<>
struct unsigned_bits_impl_t<64> {
	using type = std::uint64_t;
};

template<std::uint8_t bits_v>
using unsigned_bits_t = typename unsigned_bits_impl_t<bits_v>::type;

} // namespace sigil
