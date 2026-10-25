/***
 * @file test_mutation_round6.cpp
 * @brief Round 6 mutation-targeted exact-output tests
 */

// NOLINTBEGIN

#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "compiler.h"
#include "compiler_hooks.h"
#include "dispatch_fixup_resolver.h"
#include "doctest/doctest.h"
#include "test_support.h"

static std::string compileRound6Hex(const std::string& name,
                                    const std::string& program,
                                    bool* okOut = nullptr) {
  CompilationHarness harness;
  auto compiler = harness.compileSource("tmp/" + name + ".bas", program);
  bool ok = compiler != nullptr;
  if (okOut) *okOut = ok;
  if (!ok) return std::string();
  std::map<unsigned int, unsigned int> stubs;
  const auto& fixups = compiler->getContext()->getDispatchFixups();
  for (std::size_t i = 0; i < fixups.size(); i++) {
    stubs[fixups[i].dispatchIndex] =
        kernelImage()->dispatchAddress(fixups[i].dispatchIndex);
  }
  std::string error;
  applyDispatchFixups(fixups, stubs,
                      compiler->getContext()->getCpu()->context->getCode(),
                      error);

  int n = compiler->getCodeSize();
  std::vector<unsigned char> buffer(n);
  compiler->write(buffer.data(), 0);
  std::string out;
  char byte[4];
  for (int i = 0; i < n; i++) {
    std::snprintf(byte, sizeof(byte), "%02X", buffer[i]);
    out += byte;
  }
  return out;
}

static std::string kernelCallHex(int dispatch) {
  unsigned int address = kernelImage()->dispatchAddress(dispatch);
  char bytes[8];
  std::snprintf(bytes, sizeof(bytes), "CD%02X%02X", address & 0xFF,
                (address >> 8) & 0xFF);
  return bytes;
}

static int round6Size(const std::string& name, const std::string& program) {
  bool ok = false;
  std::string hex = compileRound6Hex(name, program, &ok);
  REQUIRE(ok == true);
  return static_cast<int>(hex.size() / 2);
}

static void checkFragment(const std::string& name, const std::string& program,
                          const std::string& fragment) {
  std::string hex = compileRound6Hex(name, program);
  INFO("program: " << program);
  INFO("hex: " << hex);
  INFO("fragment: " << fragment);
  CHECK(hex.find(fragment) != std::string::npos);
}

TEST_SUITE("Round6Graphics") {
  TEST_CASE("SET SCROLL emits exact branch fragments for 1..4 arguments") {
    checkFragment("r6_scroll1", "10 SET SCROLL 1\n20 END\n",
                  "210100E53EFF06FF21FFFFD1");
    checkFragment("r6_scroll2", "10 SET SCROLL 1,2\n20 END\n",
                  "210100E5210200E53EFF06FFE1D1");
    checkFragment("r6_scroll3", "10 SET SCROLL 1,2,3\n20 END\n",
                  "210100E5210200E521030065E53EFFC1E1D1");
    checkFragment("r6_scroll4", "10 SET SCROLL 1,2,3,4\n20 END\n",
                  "210100E5210200E521030065E521040065E5F1C1E1D1");
  }

  TEST_CASE("SET SCROLL null and truncated arguments keep distinct bytes") {
    checkFragment("r6_scroll_null", "10 SET SCROLL ,2,3,4\n20 END\n",
                  "21FFFFE5210200E521030065E521040065E5F1C1E1D1");
    CHECK(round6Size("r6_scroll_t1", "10 SET SCROLL 1\n20 END\n") == 66);
    CHECK(round6Size("r6_scroll_t2", "10 SET SCROLL 1,2\n20 END\n") == 68);
    CHECK(round6Size("r6_scroll_t3", "10 SET SCROLL 1,2,3\n20 END\n") == 72);
    CHECK(round6Size("r6_scroll_t4", "10 SET SCROLL 1,2,3,4\n20 END\n") == 76);
  }

  TEST_CASE("SET PAGE emits both arguments and skips null literals") {
    checkFragment("r6_page1", "10 SET PAGE 1\n20 END\n",
                  "2101007D" + kernelCallHex(DISP_XBASIC_SET_PAGE));
    checkFragment("r6_page2", "10 SET PAGE 1,2\n20 END\n",
                  "2102007D32F6FA");
    checkFragment("r6_page_null", "10 SET PAGE ,1\n20 END\n",
                  "2101007D32F6FA");
  }

  TEST_CASE("SET VIDEO emits one block per argument and skips null") {
    CHECK(round6Size("r6_video1", "10 SET VIDEO 1\n20 END\n") == 103);
    CHECK(round6Size("r6_video7", "10 SET VIDEO 1,2,3,4,5,6,7\n20 END\n") ==
          199);
    CHECK(round6Size("r6_video_null", "10 SET VIDEO 1,,3\n20 END\n") == 122);
    checkFragment("r6_video_first", "10 SET VIDEO 1,2,3\n20 END\n",
                  "2101007DE6033CF610E17723");
  }

  TEST_CASE("SET TILE PATTERN distinguishes buffer, inline and bank forms") {
    checkFragment("r6_pattern_buf",
                  "10 DIM PB%(3)\n20 SET TILE PATTERN 65, PB%()\n30 END\n",
                  "2141007DF52148C00603F1" +
                      kernelCallHex(DISP_set_tile_pattern_buffer));
    checkFragment("r6_pattern_buf_bank",
                  "10 DIM PB%(3)\n20 SET TILE PATTERN 65, PB%(), 2\n30 END\n",
                  "2141007DF52148C0E521020045E1F1" +
                      kernelCallHex(DISP_set_tile_pattern_buffer));
    checkFragment("r6_pattern_tuple",
                  "10 SET TILE PATTERN 65, (255,0,255,0,255,0,255,0)\n20 END\n",
                  "2141002247F826032249F8");
    checkFragment(
        "r6_pattern_tuple_bank",
        "10 SET TILE PATTERN 65, (255,0,255,0,255,0,255,0),1\n20 END\n",
        "2141002247F8210100652249F8");
  }

  TEST_CASE("SET TILE COLOR distinguishes buffer, inline and pair forms") {
    checkFragment("r6_color_buf",
                  "10 DIM CB%(7)\n20 SET TILE COLOR 5, CB%()\n30 END\n",
                  "2105007DF52148C00603F1" +
                      kernelCallHex(DISP_set_tile_color_buf));
    checkFragment("r6_color_buf_bank",
                  "10 DIM CB%(7)\n20 SET TILE COLOR 5, CB%(),1\n30 END\n",
                  "2105007DF52148C0E521010045E1F1" +
                      kernelCallHex(DISP_set_tile_color_buf));
    checkFragment("r6_color_tuple",
                  "10 SET TILE COLOR 5, (1,2,3,4)\n20 END\n",
                  "2105002247F826032249F8");
    checkFragment("r6_color_tuple_bank",
                  "10 SET TILE COLOR 5, (1,2,3,4),1\n20 END\n",
                  "2105002247F8210100652249F8");
  }

  TEST_CASE("COLOR covers numeric, null, sprite and restore forms") {
    checkFragment("r6_color1", "10 COLOR 15\n20 END\n", "210F007D32E9F3");
    checkFragment("r6_color3", "10 COLOR 15,1,2\n20 END\n",
                  "2101007D32EAF32102007D32EBF3");
    checkFragment("r6_color_null", "10 COLOR 15,,2\n20 END\n",
                  "2102007D32EBF3");
    CHECK(compileRound6Hex("r6_color_null_neg", "10 COLOR 15,,2\n20 END\n")
              .find("2101007D32EAF3") == std::string::npos);
    CHECK(round6Size("r6_color_new", "10 COLOR NEW\n20 END\n") == 58);
    CHECK(round6Size("r6_color_restore", "10 COLOR RESTORE\n20 END\n") == 58);
    CHECK(round6Size("r6_color_sprite", "10 COLOR SPRITE(0)=1\n20 END\n") ==
          57);
    CHECK(round6Size("r6_color_sprites", "10 COLOR SPRITE$(0)=1\n20 END\n") ==
          56);
  }

  TEST_CASE("SCREEN covers numeric, null and command sub-forms") {
    checkFragment("r6_screen0", "10 SCREEN 0\n20 END\n",
                  "2100007D" + kernelCallHex(DISP_XBASIC_SCREEN));
    checkFragment("r6_screen2", "10 SCREEN 0,1\n20 END\n",
                  "2100007D" + kernelCallHex(DISP_XBASIC_SCREEN) + "2101007D" +
                      kernelCallHex(DISP_XBASIC_SCREEN_SPRITE));
    checkFragment("r6_screen_null1", "10 SCREEN ,1\n20 END\n", "2101007D");
    checkFragment("r6_screen_null2", "10 SCREEN 0,\n20 END\n", "2100007D");
    CHECK(round6Size("r6_screen_scroll", "10 SCREEN SCROLL 1\n20 END\n") == 59);
    CHECK(round6Size("r6_screen_load", "10 A=1\n20 SCREEN LOAD A\n30 END\n") ==
          73);
    CHECK(round6Size("r6_screen_paste", "10 A=1\n20 SCREEN PASTE FROM A\n30 END\n") ==
          63);
  }

  TEST_CASE("COPY operator codes emit their exact immediate") {
    checkFragment("r6_copy_basic", "10 COPY (0,0)-(1,1) TO (2,2)\n20 END\n",
                  "06003AF6FA3219FCE122B9FCE122B7FC3AF6FA3218FC");
    checkFragment("r6_copy_op0", "10 COPY (0,0)-(1,1) TO (2,2),0\n20 END\n",
                  "2100007DF50600F1");
    checkFragment("r6_copy_op1", "10 COPY (0,0)-(1,1) TO (2,2),1\n20 END\n",
                  "2101007DF50600F1");
    checkFragment("r6_copy_op2", "10 COPY (0,0)-(1,1) TO (2,2),2\n20 END\n",
                  "2102007DF50600F1");
    checkFragment("r6_copy_op3", "10 COPY (0,0)-(1,1) TO (2,2),3\n20 END\n",
                  "2103007DF50600F1");
  }

  TEST_CASE("LINE, BOX and BOXF select their kernel call") {
    checkFragment("r6_line", "10 LINE (0,0)-(1,1),15\n20 END\n",
                  "210F007D0600E1D1FDE1DDE1" +
                      kernelCallHex(DISP_XBASIC_LINE));
    checkFragment("r6_line_box", "10 LINE (0,0)-(1,1),15,B\n20 END\n",
                  "210F007D0600E1D1FDE1DDE1" +
                      kernelCallHex(DISP_XBASIC_BOX));
    checkFragment("r6_line_bf", "10 LINE (0,0)-(1,1),15,BF\n20 END\n",
                  "210F007D0600E1D1FDE1DDE1" +
                      kernelCallHex(DISP_XBASIC_BOXF));
    CHECK(round6Size("r6_line_step", "10 LINE STEP(1,1)-(2,2),15\n20 END\n") ==
          97);
  }

  TEST_CASE("GET tile pattern and color emit exact lengths and addresses") {
    checkFragment("r6_get_pattern",
                  "10 DIM QB%(3)\n20 GET TILE PATTERN 67,QB%()\n30 END\n",
                  "2143007DF52148C00600F1" +
                      kernelCallHex(DISP_get_tile_pattern));
    checkFragment("r6_get_pattern_bank",
                  "10 DIM QB%(3)\n20 GET TILE PATTERN 67,QB%(),1\n30 END\n",
                  "2143007DF52148C0E521010045E1F1");
    CHECK(round6Size("r6_get_color",
                     "10 DIM G%(3)\n20 GET TILE COLOR 70,G%()\n30 END\n") ==
          58);
  }

  TEST_CASE("PUT SPRITE and PUT TILE emit exact lengths") {
    CHECK(round6Size("r6_put_sprite",
                     "10 PUT SPRITE 0,(1,2),3\n20 END\n") == 79);
    CHECK(round6Size("r6_put_sprite_short",
                     "10 PUT SPRITE 0,(1,2)\n20 END\n") == 73);
    CHECK(round6Size("r6_put_tile", "10 PUT TILE 1,(1,1)\n20 END\n") == 75);
    CHECK(round6Size("r6_put_tile_step", "10 PUT TILE 1,STEP(1,2)\n20 END\n") ==
          85);
  }

  TEST_CASE("CIRCLE, PAINT and PSET cover optional arguments") {
    checkFragment("r6_circle", "10 CIRCLE (0,0),1\n20 END\n",
                  "2101003AE9F3" + kernelCallHex(DISP_XBASIC_CIRCLE));
    checkFragment("r6_circle_color", "10 CIRCLE (0,0),1,15\n20 END\n",
                  "210100E5210F007DE1");
    CHECK(round6Size("r6_circle_aspect",
                     "10 CIRCLE (0,0),1,15,1\n20 END\n") == 109);
    checkFragment("r6_paint", "10 PAINT (0,0)\n20 END\n", "3AF2F347");
    checkFragment("r6_paint_color", "10 PAINT (0,0),15\n20 END\n",
                  "210F00457D");
    checkFragment("r6_paint_border", "10 PAINT (0,0),15,1\n20 END\n",
                  "210F0045C5210100C17DE1D1");
    checkFragment("r6_pset", "10 PSET (0,0)\n20 END\n", "3A02FB473AE9F3");
    checkFragment("r6_pset_color", "10 PSET (0,0),15\n20 END\n",
                  "210F007DF53A02FB47F1");
  }

  TEST_CASE("SPRITE LOAD and KEY forms emit exact lengths") {
    for (int i = 0; i <= 3; i++) {
      std::string name = "r6_sprite_load" + std::to_string(i);
      std::string program =
          "10 SPRITE LOAD " + std::to_string(i) + "\n20 END\n";
      CHECK(round6Size(name, program) == 53);
    }
    CHECK(round6Size("r6_sprite_on", "10 SPRITE ON\n20 END\n") == 50);
    CHECK(round6Size("r6_sprite_off", "10 SPRITE OFF\n20 END\n") == 50);
    CHECK(round6Size("r6_key_on", "10 KEY ON\n20 END\n") == 47);
    CHECK(round6Size("r6_key_off", "10 KEY OFF\n20 END\n") == 47);
    CHECK(round6Size("r6_key_list", "10 KEY 1,\"ABC\"\n20 END\n") == 89);
    CHECK(round6Size("r6_key_trap", "10 KEY 1,ON\n20 END\n") == 59);
  }

  TEST_CASE("SET keyword sub-commands emit stable lengths and calls") {
    CHECK(round6Size("r6_set_adjust", "10 SET ADJUST (1,2)\n20 END\n") ==
          125);
    CHECK(round6Size("r6_set_beep", "10 SET BEEP 1,2\n20 END\n") == 100);
    CHECK(round6Size("r6_set_date", "10 SET DATE 2025,1,2\n20 END\n") == 69);
    CHECK(round6Size("r6_set_font1", "10 SET FONT 1\n20 END\n") == 58);
    CHECK(round6Size("r6_set_font2", "10 SET FONT 1,2\n20 END\n") == 59);
    CHECK(round6Size("r6_set_prompt", "10 SET PROMPT \"X\"\n20 END\n") ==
          113);
    CHECK(round6Size("r6_set_title", "10 SET TITLE \"GAME\",1\n20 END\n") ==
          128);
    CHECK(round6Size("r6_set_tile_on", "10 SET TILE ON\n20 END\n") == 71);
    CHECK(round6Size("r6_set_tile_off", "10 SET TILE OFF\n20 END\n") == 49);
    CHECK(round6Size("r6_set_tile_flip", "10 SET TILE FLIP 0,1\n20 END\n") ==
          57);
    CHECK(round6Size("r6_set_tile_flip_bank",
                     "10 SET TILE FLIP 0,1,2\n20 END\n") == 61);
    CHECK(round6Size("r6_set_tile_rot", "10 SET TILE ROTATE 0,1\n20 END\n") ==
          57);
    CHECK(round6Size("r6_set_tile_rot_bank",
                     "10 SET TILE ROTATE 0,1,1\n20 END\n") == 61);
    CHECK(round6Size("r6_set_sprite_flip",
                     "10 SET SPRITE FLIP 0,1\n20 END\n") == 55);
    CHECK(round6Size("r6_set_sprite_rot",
                     "10 SET SPRITE ROTATE 0,1\n20 END\n") == 55);
    checkFragment("r6_set_tile_flip_frag",
                  "10 SET TILE FLIP 0,1\n20 END\n",
                  "210000E5210100D10603" + kernelCallHex(DISP_set_tile_flip));
    checkFragment("r6_set_tile_rot_frag",
                  "10 SET TILE ROTATE 0,1\n20 END\n",
                  "210000E5210100D10603" + kernelCallHex(DISP_set_tile_rotate));
    checkFragment("r6_set_sprite_flip_frag",
                  "10 SET SPRITE FLIP 0,1\n20 END\n",
                  "210000E5210100D1" + kernelCallHex(DISP_set_sprite_flip));
    checkFragment("r6_set_sprite_rot_frag",
                  "10 SET SPRITE ROTATE 0,1\n20 END\n",
                  "210000E5210100D1" + kernelCallHex(DISP_set_sprite_rotate));
  }

  TEST_CASE("SET SPRITE HITBOX modes and margins emit exact bytes") {
    checkFragment("r6_hitbox_on", "10 SET SPRITE HITBOX ON\n20 END\n",
                  "3E00" + kernelCallHex(DISP_set_sprite_hitbox));
    checkFragment("r6_hitbox_off", "10 SET SPRITE HITBOX OFF\n20 END\n",
                  "3E01" + kernelCallHex(DISP_set_sprite_hitbox));
    checkFragment("r6_hitbox_auto", "10 SET SPRITE HITBOX AUTO\n20 END\n",
                  "3E02" + kernelCallHex(DISP_set_sprite_hitbox));
    checkFragment("r6_hitbox_sprite", "10 SET SPRITE HITBOX 0\n20 END\n",
                  "2100003E03" + kernelCallHex(DISP_set_sprite_hitbox));
    checkFragment("r6_hitbox_margins",
                  "10 SET SPRITE HITBOX 1,3,2,3,2\n20 END\n",
                  "210200E5210300E5210200E5210300E52101003E04" +
                      kernelCallHex(DISP_set_sprite_hitbox));
  }

  TEST_CASE("Sentinel coordinates reach every COPY immediate position") {
    checkFragment("r6_copy_sentinel",
                  "10 COPY (1,2)-(3,4) TO (5,6)\n20 END\n",
                  "21010022B7FCE521020022B9FCE521030022B7FCE521040022B9FCE5"
                  "21050022B7FCE521060022B9FCE5");
  }
}

TEST_SUITE("Round6Control") {
  TEST_CASE("ON trap families emit exact lengths") {
    CHECK(round6Size("r6_on_key",
                     "10 ON KEY GOSUB 100,200\n20 END\n100 RETURN\n200 RETURN\n") ==
          84);
    CHECK(round6Size("r6_on_sprite",
                     "10 ON SPRITE GOSUB 100\n20 END\n100 RETURN\n") == 67);
    CHECK(round6Size("r6_on_stop",
                     "10 ON STOP GOSUB 100\n20 END\n100 RETURN\n") == 67);
    CHECK(round6Size("r6_on_interval",
                     "10 ON INTERVAL=1 GOSUB 100\n20 END\n100 RETURN\n") ==
          85);
    CHECK(round6Size("r6_on_dummy",
                     "10 ON KEY GOSUB 100,\"X\"\n20 END\n100 RETURN\n") == 80);
  }

  TEST_CASE("IF, FOR and basic assignment forms emit exact lengths") {
    CHECK(round6Size("r6_if_line", "10 IF A=1 THEN 20\n20 END\n") == 73);
    CHECK(round6Size("r6_if_goto", "10 IF A=1 THEN GOTO 20\n20 END\n") == 73);
    CHECK(round6Size("r6_if_assign", "10 IF A=1 THEN B=2\n20 END\n") == 83);
    CHECK(round6Size("r6_if_and", "10 IF A=1 AND B=2 THEN 20\n20 END\n") == 99);
    CHECK(round6Size("r6_for", "10 FOR I=1 TO 10\n20 NEXT\n") == 144);
    CHECK(round6Size("r6_for_step", "10 FOR I=1 TO 10 STEP 2\n20 NEXT\n") ==
          144);
    CHECK(round6Size("r6_let", "10 LET A=1\n20 END\n") == 57);
    CHECK(round6Size("r6_assign", "10 A=1\n20 END\n") == 57);
  }

  TEST_CASE("I/O and CMD families emit exact lengths") {
    CHECK(round6Size("r6_input", "10 INPUT A\n20 END\n") == 57);
    CHECK(round6Size("r6_input_prompt", "10 INPUT \"X\";A\n20 END\n") == 65);
    CHECK(round6Size("r6_input_file", "10 INPUT #1,A\n20 END\n") == 80);
    CHECK(round6Size("r6_print", "10 PRINT A\n20 END\n") == 57);
    CHECK(round6Size("r6_print_using", "10 PRINT USING \"##\";A\n20 END\n") ==
          89);
    CHECK(round6Size("r6_wait", "10 WAIT 1,2\n20 END\n") == 58);
    CHECK(round6Size("r6_swap", "10 SWAP A,B\n20 END\n") == 53);
    CHECK(round6Size("r6_poke", "10 POKE 1,2\n20 END\n") == 54);
    CHECK(round6Size("r6_out", "10 OUT 1,2\n20 END\n") == 54);
    CHECK(round6Size("r6_bload", "10 BLOAD \"X.SC2\",S\n20 END\n") == 53);
    for (int i = 0; i <= 9; i++) {
      std::string name = "r6_mtf" + std::to_string(i);
      std::string program = "10 CMD MTF " + std::to_string(i) + "\n20 END\n";
      CHECK(round6Size(name, program) == 86);
    }
  }

  TEST_CASE("SOUND, PLAY and remaining handlers emit exact lengths") {
    CHECK(round6Size("r6_sound2", "10 SOUND 1,2\n20 END\n") == 57);
    CHECK(round6Size("r6_play", "10 PLAY \"C\"\n20 END\n") == 59);
    CHECK(round6Size("r6_interval", "10 INTERVAL ON\n20 END\n") == 50);
    CHECK(round6Size("r6_gosub", "10 GOSUB 100\n20 END\n100 RETURN\n") ==
          48);
    CHECK(round6Size("r6_goto", "10 GOTO 100\n20 END\n100 END\n") == 50);
    CHECK(round6Size("r6_open",
                     "10 OPEN \"A.TXT\" FOR INPUT AS #1\n20 END\n") == 68);
    CHECK(round6Size("r6_close", "10 CLOSE #1\n20 END\n") == 51);
    CHECK(round6Size("r6_read", "10 READ A\n20 END\n") == 57);
    CHECK(round6Size("r6_ipoke", "10 IPOKE 1,2\n20 END\n") == 56);
    CHECK(round6Size("r6_vpoke", "10 VPOKE 1,2\n20 END\n") == 56);
    CHECK(round6Size("r6_iread", "10 IREAD A\n20 END\n") == 57);
    CHECK(round6Size("r6_cmd_page", "10 CMD PAGE 1\n20 END\n") == 55);
    CHECK(round6Size("r6_cmd_setfnt", "10 CMD SETFNT 1\n20 END\n") == 59);
    CHECK(round6Size("r6_cmd_plysound",
                     "10 CMD PLYSOUND 1,2,3\n20 END\n") == 65);
    CHECK(round6Size("r6_cmd_play", "10 CMD PLAY 1\n20 END\n") == 57);
  }
}

TEST_SUITE("Round6Semantics") {
  TEST_CASE("Integer arithmetic emits exact lengths and opcodes") {
    CHECK(round6Size("r6_neg", "10 A=-1\n20 END\n") == 60);
    checkFragment("r6_add", "10 A=1+2\n20 END\n", "11010021020019");
    checkFragment("r6_sub", "10 A=1-2\n20 END\n", "2101002B2B");
    checkFragment("r6_mul", "10 A=1*2\n20 END\n", "21010029");
    checkFragment("r6_div", "10 A=1/2\n20 END\n", "210100CB2CCB1D30067CE680280123");
    checkFragment("r6_idiv", "10 A=1\\2\n20 END\n", "210100110200");
    checkFragment("r6_mod", "10 A=1 MOD 2\n20 END\n", "210100110200");
    checkFragment("r6_pow", "10 A=1^2\n20 END\n", "210100E5210200EBE1");
  }

  TEST_CASE("Integer comparisons call their exact kernel helper") {
    checkFragment("r6_eq", "10 A=1=2\n20 END\n",
                  "110100210200" + kernelCallHex(DISP_intCompareEQ));
    checkFragment("r6_ne", "10 A=1<>2\n20 END\n",
                  "110100210200" + kernelCallHex(DISP_intCompareNE));
    checkFragment("r6_lt", "10 A=1<2\n20 END\n",
                  "110100210200" + kernelCallHex(DISP_intCompareLT));
    checkFragment("r6_gt", "10 A=1>2\n20 END\n",
                  "110100210200" + kernelCallHex(DISP_intCompareGT));
    checkFragment("r6_le", "10 A=1<=2\n20 END\n",
                  "110100210200" + kernelCallHex(DISP_intCompareLE));
    checkFragment("r6_ge", "10 A=1>=2\n20 END\n",
                  "110100210200" + kernelCallHex(DISP_intCompareGE));
  }

  TEST_CASE("Logic operators call their exact kernel helper") {
    checkFragment("r6_and", "10 A=1 AND 2\n20 END\n",
                  "110100210200" + kernelCallHex(DISP_intCompareAND));
    checkFragment("r6_or", "10 A=1 OR 2\n20 END\n",
                  "110100210200" + kernelCallHex(DISP_intCompareOR));
    checkFragment("r6_not", "10 A=NOT 1\n20 END\n",
                  "210100" + kernelCallHex(DISP_intCompareNOT));
  }

  TEST_CASE("Typed assignments and casts keep distinct exact lengths") {
    CHECK(round6Size("r6_int_pct", "10 A%=1\n20 END\n") == 50);
    CHECK(round6Size("r6_single", "10 A!=1\n20 END\n") == 57);
    CHECK(round6Size("r6_double", "10 A#=1\n20 END\n") == 57);
    CHECK(round6Size("r6_str", "10 A$=\"X\"\n20 END\n") == 58);
    CHECK(round6Size("r6_strvar", "10 A$=B$\n20 END\n") == 56);
    CHECK(round6Size("r6_cast_int", "10 A%=1.5\n20 END\n") == 55);
    CHECK(round6Size("r6_cast_double", "10 A#=1.5\n20 END\n") == 56);
    CHECK(round6Size("r6_array", "10 DIM A(2)\n20 A(0)=1\n30 END\n") ==
          62);
    CHECK(round6Size("r6_array2d", "10 DIM A(2,3)\n20 A(1,2)=3\n30 END\n") ==
          62);
  }

  TEST_CASE("IF dispatch uses the typed comparison helper") {
    checkFragment("r6_if_int", "10 IF A%=1 THEN 20\n20 END\n",
                  kernelCallHex(DISP_intCompareEQ));
    checkFragment("r6_if_single", "10 IF A!=1 THEN 20\n20 END\n",
                  kernelCallHex(DISP_XBASIC_COMPARE_FLOATS_EQ));
    checkFragment("r6_if_double", "10 IF A#=1 THEN 20\n20 END\n",
                  kernelCallHex(DISP_XBASIC_COMPARE_FLOATS_EQ));
    checkFragment("r6_if_str", "10 IF A$=\"X\" THEN 20\n20 END\n",
                  kernelCallHex(DISP_XBASIC_COMPARE_STRING_WITH_NULBUF_EQ));
  }

  TEST_CASE("Function strategies emit exact lengths") {
    CHECK(round6Size("r6_using", "10 A$=USING$(\"##\",1)\n20 END\n") == 91);
    CHECK(round6Size("r6_mid", "10 A$=MID$(\"ABC\",1,2)\n20 END\n") == 85);
    CHECK(round6Size("r6_psg", "10 A=PSG(0)\n20 END\n") == 68);
    CHECK(round6Size("r6_loc", "10 A=LOC(1)\n20 END\n") == 60);
    CHECK(round6Size("r6_vdp", "10 A=VDP(0)\n20 END\n") == 60);
    CHECK(round6Size("r6_pdl", "10 A=PDL(0)\n20 END\n") == 64);
    CHECK(round6Size("r6_base", "10 A=BASE(0)\n20 END\n") == 60);
    CHECK(round6Size("r6_turbo", "10 A=TURBO\n20 END\n") == 59);
    CHECK(round6Size("r6_maker", "10 A=MAKER\n20 END\n") == 59);
  }
}

TEST_SUITE("Round6ParserTail") {
  TEST_CASE("Parser success paths preserve exact code sizes") {
    CHECK(round6Size("r6_parse_gfx", "10 LINE (0,0)-(1,1),15,B\n20 END\n") ==
          87);
    CHECK(round6Size("r6_parse_file", "10 OPEN \"X\" AS#1\n20 END\n") ==
          64);
    CHECK(round6Size("r6_parse_data", "10 DATA 1,2,3\n20 END\n") == 53);
    CHECK(round6Size("r6_parse_def", "10 DEFINT A-Z\n20 END\n") == 44);
    CHECK(round6Size("r6_parse_print",
                     "10 PRINT USING \"##\";A\n20 END\n") == 89);
    CHECK(round6Size("r6_parse_put", "10 PUT SPRITE 0,(1,2),3\n20 END\n") ==
          79);
  }

}

// NOLINTEND
