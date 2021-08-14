project(halligalli-test)

# Build absolute path for test resources and pass it as define
get_filename_component(TEST_RES_PATH res ABSOLUTE)
add_compile_definitions(TEST_RES="${TEST_RES_PATH}")

set(test_SOURCES
	catch_main.cpp
	test_initializer.cpp
	test_network.cpp
	test_parse_lp.cpp
)

add_executable(test_catch2 ${test_SOURCES})
target_link_libraries(test_catch2 PRIVATE Catch2::Catch2 halligalli)

add_test(NAME test COMMAND test_catch2)
