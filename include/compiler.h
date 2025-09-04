/***
 * @file compiler.h
 * @brief Compiler class header for semantic analysis,
 *        specialized as a Z80 code builder for MSX system
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   https://en.wikipedia.org/wiki/Semantic_analysis_(computational)
 *   https://refactoring.guru/design-patterns/bridge
 *   Z80 Opcodes:
 *     http://z80-heaven.wikidot.com/instructions-set
 *     https://clrhome.org/table/
 *   Basic Kun Math Pack:
 *     https://www.msx.org/wiki/Category:X-BASIC#Floating_points
 */

#ifndef COMPILER_H
#define COMPILER_H

#include <string.h>

#include <queue>

#include "fswrapper.h"
#include "parse.h"
#include "pletter.h"
#include "resources.h"
#include "symbols.h"
#include "z80.h"

#define COMPILE_MAX_PAGES (16 * 4)
#define COMPILE_CODE_SIZE (COMPILE_MAX_PAGES * 0x4000)
#define COMPILE_RAM_SIZE (0xFFFF)

extern unsigned char bin_header_bin[];

/***
 * @class Compiler
 * @brief Compiler class for semantic analysis,
 * specialized as a Z80 code builder for MSX system
 */
class Compiler : public IZ80 {
 private:
  /***
   * @brief Perform a semanthic analysis on the specified tag node
   * @param tag TagNode object (action list)
   * @return True, if semanthic analysis success
   */
  bool evaluate(TagNode* tag);

  void addByteOptimized(unsigned char byte);
  void addKernelCall(unsigned int word);
  int getKernelCallAddr(unsigned int word);
  void addLdHLmegarom();

  bool evalAction(ActionNode* action);
  bool evalActions(ActionNode* action);
  int evalExpression(ActionNode* action);
  int evalOperator(ActionNode* action);
  int evalFunction(ActionNode* action);
  bool evalOperatorParms(ActionNode* action, int parmCount);
  int evalOperatorCast(ActionNode* action);

  bool addVarAddress(ActionNode* action);
  void addTempStr(bool atHL);
  void addCast(int from, int to);
  bool addAssignment(ActionNode* action);

  SymbolNode* getSymbol(Lexeme* lexeme);
  SymbolNode* addSymbol(Lexeme* lexeme);
  SymbolNode* getSymbol(TagNode* tag);
  SymbolNode* addSymbol(TagNode* tag);
  SymbolNode* addSymbol(string line);

  FixNode* addFix(Lexeme* lexeme);
  FixNode* addFix(SymbolNode* symbol);
  FixNode* addFix(string line);
  SymbolNode* addPreMark();
  FixNode* addMark();

  void func_symbols();
  void clear_symbols();
  int save_symbols();
  void do_fix();

  /***
   * @brief Convert a double to MSX float point math pack library format
   * @param value Value to convert
   * @param words 32 bits destination buffer
   * @note
   * [Basic Kun Math
   * Pack](https://www.msx.org/wiki/Category:X-BASIC#Floating_points)
   */
  void double2FloatLib(double value, int* words);

  /***
   * @brief Convert a float to MSX float point math pack library format
   * @param value Value to convert
   * @param words 16 bits destination buffer
   * @note
   * [Basic Kun Math
   * Pack](https://www.msx.org/wiki/Category:X-BASIC#Floating_points)
   */
  void float2FloatLib(float value, int* words);

  /***
   * @brief Convert a string to MSX float point math pack library format
   * @param value String to convert
   * @return 16 bits float value
   * @note
   * [Basic Kun Math
   * Pack](https://www.msx.org/wiki/Category:X-BASIC#Floating_points)
   */
  int str2FloatLib(string value);

  /***
   * @brief Convert a string to MSX PRINT USING format flags
   * @param value String to convert
   * @return 16 bits flags value
   * @example flags = getUsingFormat("###,##0.00");
   * @note
   * [PRINT USING](https://www.msx.org/wiki/PRINT#Parameters)
   */
  int getUsingFormat(string text);

  /***
   * @defgroup StatementsCompilingGroup
   * @brief Statements compiling group
   * @{
   */

  void cmd_start();
  void cmd_end(bool doCodeRegistering);
  void cmd_cls();
  void cmd_print();
  void cmd_input(bool question);
  void cmd_beep();
  void cmd_goto();
  void cmd_gosub();
  void cmd_return();
  void cmd_sound();
  void cmd_play();
  void cmd_draw();
  void cmd_let();
  void cmd_dim();
  void cmd_redim();
  void cmd_randomize();
  void cmd_if();
  void cmd_for();
  void cmd_next();
  void cmd_locate();
  void cmd_screen();
  void cmd_screen_copy();
  void cmd_screen_paste();
  void cmd_screen_scroll();
  void cmd_screen_load();
  void cmd_screen_on();
  void cmd_screen_off();
  void cmd_color();
  void cmd_width();
  void cmd_pset(bool forecolor);
  void cmd_line();
  void cmd_paint();
  void cmd_circle();
  void cmd_copy();
  void cmd_copy_screen();
  void cmd_data();
  void cmd_idata();
  void cmd_read();
  void cmd_iread();
  void cmd_restore();
  void cmd_irestore();
  void cmd_resume();
  void cmd_out();
  void cmd_poke();
  void cmd_ipoke();
  void cmd_vpoke();
  void cmd_put();
  void cmd_put_sprite();
  void cmd_put_tile();
  void cmd_set();
  void cmd_set_adjust();
  void cmd_set_page();
  void cmd_set_scroll();
  void cmd_set_video();
  void cmd_set_screen();
  void cmd_set_beep();
  void cmd_set_title();
  void cmd_set_prompt();
  void cmd_set_tile();
  void cmd_set_sprite();
  void cmd_set_font();
  void cmd_set_date();
  void cmd_set_time();
  void cmd_get();
  void cmd_get_date();
  void cmd_get_time();
  void cmd_on();
  void cmd_on_error();
  void cmd_on_interval();
  void cmd_on_key();
  void cmd_on_sprite();
  void cmd_on_stop();
  void cmd_on_strig();
  void cmd_on_goto_gosub();
  void cmd_interval();
  void cmd_stop();
  void cmd_sprite();
  void cmd_sprite_load();
  void cmd_key();
  void cmd_strig();
  void cmd_swap();
  void cmd_wait();
  void cmd_file();
  void cmd_text();
  void cmd_call();
  void cmd_cmd();
  void cmd_maxfiles();
  void cmd_open();
  void cmd_close();
  void cmd_def();
  void cmd_bload();

  /***
   * @remark End of StatementsCompilingGroup
   * @}
   */

  bool addCheckTraps();
  void addEnableBasicSlot();
  void addDisableBasicSlot();

  void beginBasicSetStmt(string name);
  void endBasicSetStmt();
  void addBasicChar(char c);

  void syntax_error();
  void syntax_error(string msg);

 public:
  Compiler();
  virtual ~Compiler();

  /***
   * @brief Perform a semanthic analysis on the parsed list
   * @param parser Parser object
   * @return True, if semanthic analysis success
   */
  bool build(Parser* parser);
  int write(unsigned char* dest, int start_address);

  float ramMemoryPerc;
  bool pt3, akm, font, file_support, has_defusr;
  bool has_open_grp;
  bool has_tiny_sprite;

  ResourceManager resourceManager;

  int code_start, ram_start, ram_page;
  int ram_size;
  int segm_last, segm_total;
  string error_message;

  TagNode* current_tag;
  Parser* parser;
  BuildOptions* opts;

  bool compiled;

 protected:
  unsigned char* ram;  //[0xFFFF];

  int ram_pointer;
  int mark_count, for_count;

  SymbolNode *heap_mark, *temp_str_mark;
  SymbolNode* end_mark;
  FixNode *enable_basic_mark, *disable_basic_mark;
  FixNode* draw_mark;
  FixNode *io_redirect_mark, *io_screen_mark;

  ActionNode* current_action;

  vector<SymbolNode*> symbols;
  vector<FixNode*> fixes;
  stack<ForNextNode*> forNextStack;
};

#endif  // COMPILER_H
