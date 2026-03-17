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

bool CompilerCmdMtfHandler::execute(CompilerContext* context,
                                    shared_ptr<ActionNode> action) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  if (!action->actions.size()) {
    context->syntaxError("CMD MTF parameters is missing");
  } else if (action->actions.size() > 4) {
    context->syntaxError("CMD MTF syntax error");
  } else {
    // resource number parameter
    auto sub = action->actions[0];
    int subtype = expression.evalExpression(sub);
    expression.addCast(subtype, Lexeme::subtype_numeric);

    if (action->actions.size() > 1) {
      cpu.addPushHL();

      // map operation parameter
      sub = action->actions[1];
      subtype = expression.evalExpression(sub);
      expression.addCast(subtype, Lexeme::subtype_numeric);

      if (action->actions.size() > 2) {
        cpu.addLdHL();
        cpu.addPushHL();

        // col/x parameter
        sub = action->actions[2];
        subtype = expression.evalExpression(sub);
        expression.addCast(subtype, Lexeme::subtype_numeric);

        if (action->actions.size() > 3) {
          cpu.addPushHL();

          // row/y parameter
          sub = action->actions[3];
          subtype = expression.evalExpression(sub);
          expression.addCast(subtype, Lexeme::subtype_numeric);

          cpu.addLdCL();
          cpu.addLdBH();   //! row/y
          cpu.addPopDE();  //! col/x
          cpu.addPopAF();  //! map operation
          cpu.addPopHL();  //! resource number
        } else {
          cpu.addExDEHL();  //! col/x
          cpu.addLdBC(0);   //! row/y
          cpu.addPopAF();   //! map operation
          cpu.addPopHL();   //! resource number
        }
      } else {
        cpu.addXorA();
        cpu.addLdEA();
        cpu.addLdDA();  //! col/x
        cpu.addLdCA();
        cpu.addLdBA();   //! row/y
        cpu.addLdAL();   //! map operation
        cpu.addPopHL();  //! resource number
      }
    } else {
      cpu.addXorA();  //! map operation
      cpu.addLdEA();
      cpu.addLdDA();  //! col/x
      cpu.addLdCA();
      cpu.addLdBA();  //! row/y
    }

    // hl = resource number
    // de = col/x position
    // bc = row/y position
    // a = map operation
    // call cmd_mtf
    cpu.addCall(def_cmd_mtf);
  }

  return context->compiled;
}
