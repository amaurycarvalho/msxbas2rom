/***
 * @file test_dispatch_stability.cpp
 * @brief Stable DISP index and generated dispatch table tests
 */

// NOLINTBEGIN

#include <string>

#include "compiler_hooks.h"
#include "doctest/doctest.h"
#include "kernel_dispatch_generated.h"

TEST_SUITE("Dispatch stability") {
  TEST_CASE("Generated dispatch table matches DISP_ENTRIES") {
    CHECK(KERNEL_DISPATCH_COUNT == DISP_ENTRIES);
    CHECK(std::string(kernel_dispatch_names[0]) == "castParamFloatInt");
    CHECK(std::string(kernel_dispatch_names[25]) == "cmd_mtf");
    CHECK(std::string(kernel_dispatch_names[DISP_MR_CALL]) == "MR_CALL");
    CHECK(std::string(kernel_dispatch_names[DISP_MR_GET_BYTE]) ==
          "MR_GET_BYTE");
    CHECK(std::string(kernel_dispatch_names[DISP_MR_GET_DATA]) ==
          "MR_GET_DATA");
    CHECK(std::string(kernel_dispatch_names[DISP_MR_JUMP]) == "MR_JUMP");
    CHECK(std::string(kernel_dispatch_names[DISP_XBASIC_INIT]) ==
          "XBASIC_INIT");
    CHECK(std::string(kernel_dispatch_names[DISP_XBASIC_SIN]) ==
          "XBASIC_SIN");
    CHECK(std::string(kernel_dispatch_names[DISP_XBASIC_PRINT_TAB]) ==
          "XBASIC_PRINT_TAB");
    CHECK(std::string(kernel_dispatch_names[DISP_set_sprite_hitbox]) ==
          "set_sprite_hitbox");
  }

  TEST_CASE("DISP indices stay pinned across relayouts") {
    CHECK(DISP_castParamFloatInt == 0);
    CHECK(DISP_cmd_mtf == 25);
    CHECK(DISP_MR_CALL == 86);
    CHECK(DISP_MR_GET_BYTE == 89);
    CHECK(DISP_MR_GET_DATA == 90);
    CHECK(DISP_MR_JUMP == 91);
    CHECK(DISP_XBASIC_INIT == 98);
    CHECK(DISP_XBASIC_SIN == 132);
    CHECK(DISP_XBASIC_PRINT_TAB == 202);
    CHECK(DISP_set_sprite_hitbox == 224);
    CHECK(DISP_ENTRIES == 225);
  }
}

// NOLINTEND
