/***
 * @file compiler_pt3.h
 * @brief Compiler class header for semantic analysis (PT3 support),
 *        specialized as a Z80 code builder for MSX system
 * @author Amaury Carvalho (2019-2020)
 * @note
 *   https://en.wikipedia.org/wiki/Semantic_analysis_(computational)
 *   https://refactoring.guru/design-patterns/bridge
 *   Z80 Opcodes:
 *     http://z80-heaven.wikidot.com/instructions-set
 *     https://clrhome.org/table/
 * @deprecated PT3 support is deprecated (not more future updates).
 */

#ifndef COMPILER_PT3_H
#define COMPILER_PT3_H

#include "compiler.h"

/***
 * @class CompilerPT3
 * @brief Compiler class for semantic analysis (PT3 support),
 * specialized as a Z80 code builder for MSX system
 * @deprecated PT3 support is deprecated (not more future updates).
 */
class CompilerPT3 : public Compiler {
  bool evaluate(TagNode *tag);

  void addByte(char byte);
  void addWord(int word);
  void addWord(char byte1, char byte2);
  void addCmd(char byte, int word);

  bool evalAction(ActionNode *action);
  bool evalActions(ActionNode *action);
  int evalExpression(ActionNode *action);
  int evalOperator(ActionNode *action);
  int evalFunction(ActionNode *action);
  bool evalOperatorParms(ActionNode *action, int parmCount);
  int evalOperatorCast(ActionNode *action);

  bool addVarAddress(ActionNode *action);
  void addTempStr(bool atHL);
  void addCast(int from, int to);
  bool addAssignment(ActionNode *action);

  SymbolNode *getSymbol(Lexeme *lexeme);
  SymbolNode *addSymbol(Lexeme *lexeme);
  SymbolNode *getSymbol(TagNode *tag);
  SymbolNode *addSymbol(TagNode *tag);
  SymbolNode *addSymbol(string line);

  FixNode *addFix(Lexeme *lexeme);
  FixNode *addFix(SymbolNode *symbol);
  FixNode *addFix(string line);
  SymbolNode *addPreMark();
  FixNode *addMark();

  FixNode *end_mark2;

  void func_symbols();
  void clear_symbols();
  int save_symbols();
  void do_fix();

  void double2FloatLib(double value, int *words);
  void float2FloatLib(float value, int *words);
  int str2FloatLib(string value);

  void cmd_start();
  void cmd_end(bool last);
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
  void cmd_color();
  void cmd_width();
  void cmd_pset(bool forecolor);
  void cmd_line();
  void cmd_paint();
  void cmd_circle();
  void cmd_copy();
  void cmd_data();
  void cmd_idata();
  void cmd_read();
  void cmd_iread();
  void cmd_restore();
  void cmd_out();
  void cmd_poke();
  void cmd_vpoke();
  void cmd_put();
  void cmd_put_sprite();
  void cmd_put_tile();
  void cmd_set();
  void cmd_set_page();
  void cmd_set_scroll();
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

  bool addCheckTraps();
  void addEnableBasicSlot();
  void addDisableBasicSlot();

  void syntax_error();
  void syntax_error(string msg);

 public:
  CompilerPT3();
  virtual ~CompilerPT3();

  bool build(Parser *parser);
  int write(unsigned char *dest, int start_address);
};

#endif  // COMPILER_PT3_H
