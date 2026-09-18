#include <sigil/sigil.hxx>

#include <gtest/gtest.h>

#include <algorithm>
#include <initializer_list>
#include <vector>

namespace sigil::tests {

class scanner_test : public testing::Test {
protected:
	void SetUp( ) override { m_buffer.assign( 4096, std::uint8_t{ 0xcc } ); }

protected:
	std::uint8_t* begin( ) { return m_buffer.data( ); }

	std::size_t size( ) const { return m_buffer.size( ); }

protected:
	void place( std::size_t offset, std::initializer_list<std::uint8_t> bytes ) {
		std::copy( bytes.begin( ), bytes.end( ), m_buffer.begin( ) + static_cast< std::ptrdiff_t >( offset ) );
	}

protected:
	std::vector<std::uint8_t> m_buffer{ };

};

TEST_F( scanner_test, finds_exact_bytes ) {
	place( 1337, { 0x48, 0x8b, 0x05, 0x11, 0x22 } );

	EXPECT_EQ( byte_scanner<"48 8B 05 11 22">.search( begin( ), size( ) ), begin( ) + 1337 );
}

TEST_F( scanner_test, returns_null_when_absent ) {
	EXPECT_EQ( byte_scanner<"48 8B 05 11 22">.search( begin( ), size( ) ), nullptr );
}

TEST_F( scanner_test, returns_first_of_several_matches ) {
	place( 300, { 0xde, 0xad, 0xbe, 0xef } );
	place( 100, { 0xde, 0xad, 0xbe, 0xef } );

	EXPECT_EQ( byte_scanner<"DE AD BE EF">.search( begin( ), size( ) ), begin( ) + 100 );
}

TEST_F( scanner_test, skips_wildcard_bytes ) {
	place( 2000, { 0xe8, 0x01, 0x02, 0x03, 0x04, 0x84, 0xc0 } );

	EXPECT_EQ( byte_scanner<"E8 ?? ?? ?? ?? 84 C0">.search( begin( ), size( ) ), begin( ) + 2000 );
	EXPECT_EQ( byte_scanner<"E8 ? ? ? ? 84 C0">.search( begin( ), size( ) ), begin( ) + 2000 );
}

TEST_F( scanner_test, matches_nibble_wildcards ) {
	place( 512, { 0x4c, 0x8b, 0x1f, 0x90 } );

	EXPECT_EQ( byte_scanner<"4? 8B ?F 90">.search( begin( ), size( ) ), begin( ) + 512 );
	EXPECT_EQ( byte_scanner<"5? 8B ?F 90">.search( begin( ), size( ) ), nullptr );
	EXPECT_EQ( byte_scanner<"4? 8B ?E 90">.search( begin( ), size( ) ), nullptr );
}

TEST_F( scanner_test, leading_and_trailing_wildcards ) {
	place( 700, { 0x11, 0x22, 0x33 } );

	EXPECT_EQ( byte_scanner<"?? 11 22 33 ??">.search( begin( ), size( ) ), begin( ) + 699 );
}

TEST_F( scanner_test, single_byte_pattern ) {
	place( 4000, { 0xc3 } );

	EXPECT_EQ( byte_scanner<"C3">.search( begin( ), size( ) ), begin( ) + 4000 );
}

TEST_F( scanner_test, wildcard_only_pattern_matches_at_begin ) {
	EXPECT_EQ( byte_scanner<"?? ??">.search( begin( ), size( ) ), begin( ) );
}

TEST_F( scanner_test, applies_result_offset ) {
	place( 1024, { 0xe8, 0x01, 0x02, 0x03, 0x04, 0x84, 0xc0 } );

	EXPECT_EQ( ( byte_scanner<"E8 ?? ?? ?? ?? 84 C0", 1>.search( begin( ), size( ) ) ), begin( ) + 1025 );
	EXPECT_EQ( ( byte_scanner<"E8 ?? ?? ?? ?? 84 C0", -4>.search( begin( ), size( ) ) ), begin( ) + 1020 );
}

TEST_F( scanner_test, finds_match_at_buffer_start ) {
	place( 0, { 0x0f, 0x1f, 0x44 } );

	EXPECT_EQ( byte_scanner<"0F 1F 44">.search( begin( ), size( ) ), begin( ) );
}

TEST_F( scanner_test, finds_match_at_buffer_end ) {
	place( size( ) - 3, { 0x0f, 0x1f, 0x44 } );

	EXPECT_EQ( byte_scanner<"0F 1F 44">.search( begin( ), size( ) ), begin( ) + size( ) - 3 );
}

TEST_F( scanner_test, ignores_match_crossing_buffer_end ) {
	place( size( ) - 3, { 0x0f, 0x1f, 0x44 } );

	EXPECT_EQ( byte_scanner<"0F 1F 44">.search( begin( ), size( ) - 1 ), nullptr );
}

TEST_F( scanner_test, buffer_smaller_than_simd_block ) {
	m_buffer.assign( 20, std::uint8_t{ 0xcc } );
	place( 9, { 0xaa, 0xbb, 0xcc, 0xdd } );

	EXPECT_EQ( byte_scanner<"AA BB CC DD">.search( begin( ), size( ) ), begin( ) + 9 );
}

TEST_F( scanner_test, buffer_smaller_than_pattern ) {
	m_buffer.assign( 3, std::uint8_t{ 0xaa } );

	EXPECT_EQ( byte_scanner<"AA AA AA AA">.search( begin( ), size( ) ), nullptr );
}

TEST_F( scanner_test, empty_buffer ) {
	EXPECT_EQ( byte_scanner<"AA BB">.search( begin( ), std::size_t{ 0 } ), nullptr );
}

TEST_F( scanner_test, accepts_pointer_range ) {
	place( 64, { 0x12, 0x34, 0x56 } );

	EXPECT_EQ( byte_scanner<"12 34 56">.search( begin( ), begin( ) + size( ) ), begin( ) + 64 );
}

TEST_F( scanner_test, accepts_container ) {
	place( 64, { 0x12, 0x34, 0x56 } );

	EXPECT_EQ( byte_scanner<"12 34 56">.search( m_buffer ), begin( ) + 64 );
}

TEST_F( scanner_test, accepts_const_bytes ) {
	place( 64, { 0x12, 0x34, 0x56 } );

	const std::uint8_t* const data{ m_buffer.data( ) };

	EXPECT_EQ( byte_scanner<"12 34 56">.search( data, size( ) ), data + 64 );
}

TEST_F( scanner_test, action_receives_every_match ) {
	const std::vector<std::size_t> offsets{ 0, 31, 32, 33, 1000, 4092 };
	for ( const std::size_t offset : offsets ) {
		place( offset, { 0xfa, 0xce, 0xb0, 0x0c } );
	}

	std::vector<std::size_t> found{ };
	byte_scanner<"FA CE B0 0C">.search( begin( ), size( ), [ & ] ( std::uint8_t* const match ) {
		found.push_back( static_cast< std::size_t >( match - begin( ) ) );
	} );

	EXPECT_EQ( found, offsets );
}

TEST_F( scanner_test, action_is_not_called_without_matches ) {
	std::size_t calls{ };
	byte_scanner<"FA CE B0 0C">.search( begin( ), size( ), [ & ] ( std::uint8_t* const ) { ++calls; } );

	EXPECT_EQ( calls, 0u );
}

TEST_F( scanner_test, unbounded_search_stops_at_first_match ) {
	place( 2048, { 0xba, 0xad, 0xf0, 0x0d } );

	EXPECT_EQ( byte_scanner<"BA AD F0 0D">.search_unbounded( begin( ) ), begin( ) + 2048 );
}

} // namespace sigil::tests
