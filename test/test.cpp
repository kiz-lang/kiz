

#define CATCH_CONFIG_MAIN 
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>


TEST_CASE("FFI Unsafe Call External Function", "[ffi][unsafe]") {
    SECTION("合法参数调用") {
        uint32_t ret = unsafe_ffi_call(0, "test_func");
        REQUIRE(ret == 0);
    }

    SECTION("非法空函数名") {
        REQUIRE_THROWS(unsafe_ffi_call(1, ""));
    }
}
