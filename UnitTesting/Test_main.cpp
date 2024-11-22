#define CATCH_CONFIG_MAIN // This defines the main function for Catch2
#include <catch2/catch_all.hpp>

TEST_CASE("Simple math test") {
    REQUIRE(2 + 2 == 4);
    REQUIRE(3 * 3 == 9);
}

TEST_CASE("String test") {
    std::string hello = "Hello, World!";
    REQUIRE(hello.length() == 13);
}
