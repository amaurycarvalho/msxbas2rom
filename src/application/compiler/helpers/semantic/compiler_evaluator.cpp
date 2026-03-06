/***
 * @file compiler_evaluator.cpp
 * @brief Compiler evaluator
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_evaluator.h"

#include "action_node.h"
#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "compiler_statement_strategy.h"
#include "tag_node.h"

CompilerEvaluator::CompilerEvaluator(CompilerContext* context) {
  this->context = context;
}

bool CompilerEvaluator::evaluate(TagNode* tag) {
  ActionNode* action;
  unsigned int i, t = tag->actions.size(), lin;

  context->current_tag = tag;

  if (context->opts->lineNumber) {
    try {
      lin = stoi(tag->name);
    } catch (exception& e) {
      printf("Warning: error while converting numeric constant %s\n",
             tag->name.c_str());
      lin = 0;
    }
    // ld hl, line number
    context->cpu->addLdHL(lin);
    // ld (CURLIN), hl
    context->cpu->addLdiiHL(def_CURLIN);
  }

  for (i = 0; i < t && context->compiled; i++) {
    action = tag->actions[i];

    if (!evalAction(action)) return false;
  }

  return context->compiled;
}

bool CompilerEvaluator::evalActions(ActionNode* action) {
  ActionNode* sub_action;
  unsigned int i, t = action->actions.size();

  for (i = 0; i < t && context->compiled; i++) {
    sub_action = action->actions[i];

    if (!evalAction(sub_action)) return false;
  }

  return context->compiled;
}

bool CompilerEvaluator::evalAction(ActionNode* action) {
  Lexeme* lexeme;
  ICompilerStatementStrategy* strategy;

  if (!action) {
    context->syntaxError();
    return false;
  }

  context->current_action = action;
  lexeme = action->lexeme;
  if (!lexeme || lexeme->type != Lexeme::type_keyword) {
    context->syntaxError();
    return false;
  }

  strategy = statementStrategyFactory.getStrategyByKeyword(lexeme->name);
  if (!strategy) {
    context->syntaxError();
    return false;
  }

  context->traps_checked = false;
  context->skip_post_trap_check = false;

  if (!strategy->execute(context)) {
    if (context->error_message.empty()) context->syntaxError();
    return false;
  }

  if (!context->skip_post_trap_check)
    if (!context->traps_checked) {
      context->codeHelper->addCheckTraps();
    }

  return context->compiled;
}

/*
bool CompilerEvaluator::dispatchStatementCommand(CompilerCommandId command,
                                                 bool& traps_checked) {
  switch (command) {
    case CompilerCommandId::end_stmt:
      traps_checked = addCheckTraps();
      cmd_end(false);
      break;
    case CompilerCommandId::clear_stmt:
      cmd_clear();
      break;
    case CompilerCommandId::def_stmt:
      cmd_def();
      break;
    case CompilerCommandId::cls_stmt:
      cmd_cls();
      break;
    case CompilerCommandId::beep_stmt:
      cmd_beep();
      break;
    case CompilerCommandId::print_stmt:
      cmd_print();
      break;
    case CompilerCommandId::input_stmt:
      cmd_input(true);
      break;
    case CompilerCommandId::goto_stmt:
      traps_checked = addCheckTraps();
      cmd_goto();
      break;
    case CompilerCommandId::gosub_stmt:
      traps_checked = addCheckTraps();
      cmd_gosub();
      break;
    case CompilerCommandId::return_stmt:
      traps_checked = addCheckTraps();
      cmd_return();
      break;
    case CompilerCommandId::sound_stmt:
      cmd_sound();
      break;
    case CompilerCommandId::out_stmt:
      cmd_out();
      break;
    case CompilerCommandId::poke_stmt:
      cmd_poke();
      break;
    case CompilerCommandId::vpoke_stmt:
      cmd_vpoke();
      break;
    case CompilerCommandId::ipoke_stmt:
      cmd_ipoke();
      break;
    case CompilerCommandId::play_stmt:
      cmd_play();
      break;
    case CompilerCommandId::draw_stmt:
      cmd_draw();
      break;
    case CompilerCommandId::let_stmt:
      cmd_let();
      break;
    case CompilerCommandId::if_stmt:
      traps_checked = addCheckTraps();
      cmd_if();
      break;
    case CompilerCommandId::for_stmt:
      cmd_for();
      break;
    case CompilerCommandId::next_stmt:
      traps_checked = addCheckTraps();
      cmd_next();
      break;
    case CompilerCommandId::dim_stmt:
      cmd_dim();
      break;
    case CompilerCommandId::redim_stmt:
      cmd_redim();
      break;
    case CompilerCommandId::randomize_stmt:
      cmd_randomize();
      break;
    case CompilerCommandId::locate_stmt:
      cmd_locate();
      break;
    case CompilerCommandId::screen_stmt:
      cmd_screen();
      break;
    case CompilerCommandId::width_stmt:
      cmd_width();
      break;
    case CompilerCommandId::color_stmt:
      cmd_color();
      break;
    case CompilerCommandId::pset_stmt:
      cmd_pset(true);
      break;
    case CompilerCommandId::preset_stmt:
      cmd_pset(false);
      break;
    case CompilerCommandId::line_stmt:
      cmd_line();
      break;
    case CompilerCommandId::paint_stmt:
      cmd_paint();
      break;
    case CompilerCommandId::circle_stmt:
      cmd_circle();
      break;
    case CompilerCommandId::copy_stmt:
      cmd_copy();
      break;
    case CompilerCommandId::put_stmt:
      cmd_put();
      break;
    case CompilerCommandId::data_stmt:
      cmd_data();
      break;
    case CompilerCommandId::idata_stmt:
      cmd_idata();
      break;
    case CompilerCommandId::read_stmt:
      cmd_read();
      break;
    case CompilerCommandId::iread_stmt:
      cmd_iread();
      break;
    case CompilerCommandId::restore_stmt:
      cmd_restore();
      break;
    case CompilerCommandId::irestore_stmt:
      cmd_irestore();
      break;
    case CompilerCommandId::resume_stmt:
      cmd_resume();
      break;
    case CompilerCommandId::get_stmt:
      cmd_get();
      break;
    case CompilerCommandId::set_stmt:
      cmd_set();
      break;
    case CompilerCommandId::on_stmt:
      traps_checked = addCheckTraps();
      cmd_on();
      break;
    case CompilerCommandId::interval_stmt:
      cmd_interval();
      break;
    case CompilerCommandId::key_stmt:
      cmd_key();
      break;
    case CompilerCommandId::strig_stmt:
      cmd_strig();
      break;
    case CompilerCommandId::sprite_stmt:
      cmd_sprite();
      break;
    case CompilerCommandId::stop_stmt:
      traps_checked = addCheckTraps();
      cmd_stop();
      break;
    case CompilerCommandId::wait_stmt:
      cmd_wait();
      break;
    case CompilerCommandId::swap_stmt:
      cmd_swap();
      break;
    case CompilerCommandId::call_stmt:
      cmd_call();
      break;
    case CompilerCommandId::cmd_stmt:
      cmd_cmd();
      break;
    case CompilerCommandId::maxfiles_stmt:
      cmd_maxfiles();
      break;
    case CompilerCommandId::open_stmt:
      cmd_open();
      break;
    case CompilerCommandId::open_grp_stmt:
      context->has_open_grp = true;
      break;
    case CompilerCommandId::close_stmt:
      cmd_close();
      break;
    case CompilerCommandId::file_stmt:
      cmd_file();
      break;
    case CompilerCommandId::text_stmt:
      cmd_text();
      break;
    case CompilerCommandId::bload_stmt:
      cmd_bload();
      break;
    default:
      return false;
  }

  return context->compiled;
}

*/
