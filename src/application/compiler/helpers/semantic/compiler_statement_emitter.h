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
  void cmd_end(bool doCodeRegistering);
  void cmd_print();
  void cmd_input(bool question);
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
  void cmd_get_tile();
  void cmd_get_sprite();
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
  void cmd_open();
  void cmd_close();
  void cmd_bload();

  CompilerStatementEmitter(CompilerContext* context) : context(context) {};
};

#endif  // COMPILER_STATEMENT_EMITTER_H