add_executable(sigil_tests
  pattern_test.cxx
  scanner_test.cxx)
target_link_libraries(sigil_tests PRIVATE
  sigil::sigil
  GTest::gtest_main)
sigil_apply_target_defaults(sigil_tests)
set_target_properties(sigil_tests PROPERTIES FOLDER "tests")

include(GoogleTest)
gtest_discover_tests(sigil_tests DISCOVERY_MODE PRE_TEST)
