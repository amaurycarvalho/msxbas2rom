/***
 * @file compiler_cmd_mtf_handler.cpp
 * @brief MSX Tile Forge handler
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_cmd_mtf_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

// MTF parameters workarea offsets (kernel reads all params from RAM)
#define def_MTF_PARM def_PARM1
/// @brief Resource number
#define def_MTF_RESN_PARM def_MTF_PARM
/// @brief Horizontal screen position / Source X coordinate in the map
#define def_MTF_COLX_PARM (def_MTF_PARM + 2)
/// @brief Vertical screen position / Source Y coordinate in the map
#define def_MTF_ROWY_PARM (def_MTF_PARM + 4)
/// @brief Operation
/// @note 0=Palette/Tileset/Map full screen copy, 1=full copy with
/// absolute coord, 2=windowed partial copy
#define def_MTF_OPER_PARM (def_MTF_PARM + 6)
/// @brief Window width in tiles
#define def_MTF_WIN_W_PARM (def_MTF_PARM + 8)
/// @brief Window height in tiles
#define def_MTF_WIN_H_PARM (def_MTF_PARM + 10)
/// @brief Destination X coordinate on screen
#define def_MTF_SCR_X_PARM (def_MTF_PARM + 12)
/// @brief Destination Y coordinate on screen
#define def_MTF_SCR_Y_PARM (def_MTF_PARM + 14)
/// @brief Destination screen page
/// @note default=0, for screen 4 only
#define def_MTF_PAGE_PARM (def_MTF_PARM + 16)

bool CompilerCmdMtfHandler::execute(shared_ptr<CompilerContext> context,
                                    shared_ptr<ActionNode> action) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  bool isARegNotZero[1];
  bool isHLRegNotZero[2];

  int n = action->actions.size();

  if (!n) {
    context->syntaxError("CMD MTF parameters is missing");
    return context->compiled;
  }
  if (n > 9) {
    context->syntaxError("CMD MTF syntax error");
    return context->compiled;
  }

  // Param 1: resource number (always present, stored as word)
  auto sub = action->actions[0];
  int subtype = expression.evalExpression(sub);
  expression.addCast(subtype, Lexeme::subtype_numeric);
  cpu.addLdiiHL(def_MTF_RESN_PARM);

  // Param 2: operation (stored as byte)
  if ((isARegNotZero[0] = (n >= 2))) {
    sub = action->actions[1];
    subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);
    cpu.addLdAL();
  } else {
    cpu.addXorA();
  }
  cpu.addLdiiA(def_MTF_OPER_PARM);

  // Param 9: page (byte, default 0)
  if ((isARegNotZero[1] = (n >= 9))) {
    sub = action->actions[8];
    subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);
    cpu.addLdAL();
  } else {
    if (isARegNotZero[0]) cpu.addXorA();
  }
  cpu.addLdiiA(def_MTF_PAGE_PARM);

  // Param 3: col_x / map_x (word, default 0)
  if ((isHLRegNotZero[0] = (n >= 3))) {
    sub = action->actions[2];
    subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);
  } else {
    if (isARegNotZero[1])
      cpu.addLdHL(0);
    else {
      cpu.addLdLA();
      cpu.addLdHA();
    }
  }
  cpu.addLdiiHL(def_MTF_COLX_PARM);

  // Param 4: row_y / map_y (word, default 0)
  if ((isHLRegNotZero[1] = (n >= 4))) {
    sub = action->actions[3];
    subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);
  } else {
    if (isHLRegNotZero[0]) cpu.addLdHL(0);
  }
  cpu.addLdiiHL(def_MTF_ROWY_PARM);

  // Param 7: screen_x (word, default 0)
  if ((isHLRegNotZero[2] = (n >= 7))) {
    sub = action->actions[6];
    subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);
  } else {
    if (isHLRegNotZero[1]) cpu.addLdHL(0);
  }
  cpu.addLdiiHL(def_MTF_SCR_X_PARM);

  // Param 8: screen_y (word, default 0)
  if (n >= 8) {
    sub = action->actions[7];
    subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);
  } else {
    if (isHLRegNotZero[2]) cpu.addLdHL(0);
  }
  cpu.addLdiiHL(def_MTF_SCR_Y_PARM);

  // Param 5: width (word, default 32)
  if (n >= 5) {
    sub = action->actions[4];
    subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);
  } else {
    cpu.addLdHL(32);
  }
  cpu.addLdiiHL(def_MTF_WIN_W_PARM);

  // Param 6: height (word, default 24)
  if (n >= 6) {
    sub = action->actions[5];
    subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);
  } else {
    cpu.addLdHL(24);
  }
  cpu.addLdiiHL(def_MTF_WIN_H_PARM);

  context->codeOptimizer->addKernelCall(DISP_cmd_mtf);

  return context->compiled;
}
