#include <sigil/pattern.hxx>

#include <gtest/gtest.h>

namespace sigil::tests {

TEST( pattern_parse, counts_elements ) {
	static constexpr auto elements{ parse_pattern_string<"48 8B ?? 05 ? 4? ?F">( ) };

	static_assert( elements.size( ) == 7 );
	EXPECT_EQ( elements.size( ), 7 );
}

TEST( pattern_parse, empty_string_has_no_elements ) {
	static constexpr auto elements{ parse_pattern_string<"">( ) };

	static_assert( elements.size( ) == 0 );
	EXPECT_EQ( elements.size( ), 0 );
}

TEST( pattern_parse, full_byte ) {
	static constexpr auto elements{ parse_pattern_string<"8B">( ) };

	static_assert( elements[ 0 ] == element_t{ std::uint8_t{ 0x8b } } );
	ASSERT_TRUE( elements[ 0 ].m_byte.has_value( ) );
	EXPECT_EQ( *elements[ 0 ].m_byte, 0x8b );
	EXPECT_EQ( *elements[ 0 ].m_low, 0x0b );
	EXPECT_EQ( *elements[ 0 ].m_high, 0x08 );
}

TEST( pattern_parse, lower_and_upper_case_are_equal ) {
	static constexpr auto lower{ parse_pattern_string<"de ad be ef">( ) };
	static constexpr auto upper{ parse_pattern_string<"DE AD BE EF">( ) };

	for ( std::size_t i{ }; i != 4; ++i ) {
		EXPECT_TRUE( lower[ static_cast< std::uint8_t >( i ) ] == upper[ static_cast< std::uint8_t >( i ) ] );
	}
}

TEST( pattern_parse, single_and_double_wildcards_are_equal ) {
	static constexpr auto elements{ parse_pattern_string<"? ??">( ) };

	EXPECT_TRUE( elements[ 0 ] == element_t{ } );
	EXPECT_TRUE( elements[ 1 ] == element_t{ } );
	EXPECT_FALSE( elements[ 0 ].m_byte.has_value( ) );
	EXPECT_FALSE( elements[ 0 ].m_low.has_value( ) );
	EXPECT_FALSE( elements[ 0 ].m_high.has_value( ) );
}

TEST( pattern_parse, high_nibble_wildcard ) {
	static constexpr auto elements{ parse_pattern_string<"?F">( ) };

	EXPECT_FALSE( elements[ 0 ].m_byte.has_value( ) );
	EXPECT_FALSE( elements[ 0 ].m_high.has_value( ) );
	ASSERT_TRUE( elements[ 0 ].m_low.has_value( ) );
	EXPECT_EQ( *elements[ 0 ].m_low, 0x0f );
}

TEST( pattern_parse, low_nibble_wildcard ) {
	static constexpr auto elements{ parse_pattern_string<"4?">( ) };

	EXPECT_FALSE( elements[ 0 ].m_byte.has_value( ) );
	EXPECT_FALSE( elements[ 0 ].m_low.has_value( ) );
	ASSERT_TRUE( elements[ 0 ].m_high.has_value( ) );
	EXPECT_EQ( *elements[ 0 ].m_high, 0x04 );
}

TEST( pattern_sequence, groups_bytes_into_wide_comparisons ) {
	using sequence = decltype( make_byte_pattern<"01 02 03 04 05 06 07 08 ?? 09 0A 0B">( ) );

	static_assert( sequence::m_num_elements == 12 );
	ASSERT_EQ( sequence::m_bytes_entries.size( ), sizeof( void* ) >= 8 ? 3 : 4 );
	EXPECT_EQ( sequence::m_bytes_entries[ 0 ].m_size, sizeof( void* ) >= 8 ? 8 : 4 );
	EXPECT_EQ( sequence::m_nibble_entries.size( ), 0 );
}

TEST( pattern_sequence, tracks_nibble_comparisons ) {
	using sequence = decltype( make_byte_pattern<"4? ?? ?F 90">( ) );

	EXPECT_EQ( sequence::m_bytes_entries.size( ), 1 );
	ASSERT_EQ( sequence::m_nibble_entries.size( ), 2 );
	EXPECT_EQ( sequence::m_nibble_entries[ 0 ].m_offset, 0u );
	EXPECT_EQ( sequence::m_nibble_entries[ 1 ].m_offset, 2u );
}

TEST( pattern_sequence, matches_bytes_wildcards_and_nibbles ) {
	using sequence = decltype( make_byte_pattern<"48 ?? 4? ?F 05">( ) );

	const std::uint8_t matching[]{ 0x48, 0xaa, 0x4c, 0x1f, 0x05 };
	const std::uint8_t wrong_byte[]{ 0x49, 0xaa, 0x4c, 0x1f, 0x05 };
	const std::uint8_t wrong_high[]{ 0x48, 0xaa, 0x5c, 0x1f, 0x05 };
	const std::uint8_t wrong_low[]{ 0x48, 0xaa, 0x4c, 0x1e, 0x05 };

	EXPECT_TRUE( sequence::is_match( matching ) );
	EXPECT_FALSE( sequence::is_match( wrong_byte ) );
	EXPECT_FALSE( sequence::is_match( wrong_high ) );
	EXPECT_FALSE( sequence::is_match( wrong_low ) );
}

TEST( pattern_optional, compares_by_state_and_value ) {
	constexpr optional_t<std::uint8_t> empty{ };
	constexpr optional_t<std::uint8_t> five{ std::uint8_t{ 5 } };
	constexpr optional_t<std::uint8_t> six{ std::uint8_t{ 6 } };

	static_assert( empty == empty );
	static_assert( five == five );
	static_assert( five != six );
	static_assert( five != empty );
	static_assert( five == std::uint8_t{ 5 } );

	EXPECT_FALSE( empty.has_value( ) );
	EXPECT_TRUE( five.has_value( ) );
	EXPECT_EQ( *six, 6 );
}

} // namespace sigil::tests
