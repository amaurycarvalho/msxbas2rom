/***
 * @file compiler_statement_emitter.h
 * @brief Compiler statement emitter
 */

#ifndef COMPILER_STATEMENT_EMITTER_H
#define COMPILER_STATEMENT_EMITTER_H

class CompilerContext;

class CompilerStatementEmitter {
 private:
  CompilerContext* context;

 public:
  void cmd_start();
  void cmd_print();
  void cmd_sound();
  void cmd_width();
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
  void cmd_sprite();
  void cmd_sprite_load();
  void cmd_strig();
  void cmd_swap();
  void cmd_wait();
  void cmd_file();
  void cmd_text();
  void cmd_call();
  void cmd_cmd();
  void cmd_open();
  void cmd_close();
  void cmd_bload();

  CompilerStatementEmitter(CompilerContext* context) : context(context) {};
};

#endif  // COMPILER_STATEMENT_EMITTER_H