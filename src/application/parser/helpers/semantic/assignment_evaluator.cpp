/***
 * @file assignment_evaluator.cpp
 * @brief Assignments syntatic analysis class code
 * @author Amaury Carvalho (2019-2026)
 */

#include "assignment_evaluator.h"

AssignmentEvaluator::AssignmentEvaluator(ParserContext& context,
                                         ExpressionEvaluator& exprEval)
    : ctx(context), exprEval(exprEval) {}

bool AssignmentEvaluator::evaluate(LexerLine* assignment) {
  Lexeme *lexeme = assignment->getNextLexeme(), *next_lexeme;
  LexerLine parm;
  ActionNode* action;
  Lexeme* lexLet;
  bool result, add_let_action = true;

  if (lexeme) {
    lexeme = ctx.coalesceSymbols(lexeme);

    if (lexeme->type == Lexeme::type_identifier ||
        (lexeme->type == Lexeme::type_keyword &&
         lexeme->subtype == Lexeme::subtype_function)) {
      if (ctx.actionRoot) {
        lexLet = ctx.actionRoot->lexeme;
        if (lexLet->isKeyword("LET")) {
          add_let_action = false;
        }
      }

      if (add_let_action) {
        action = new ActionNode("LET");
        /// @note "lexLet" value really needs to be updated?
        /// NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
        // lexLet = action->lexeme;
        ctx.pushActionRoot(action);
      }

      parm.clearLexemes();
      parm.addLexeme(lexeme);
      while ((next_lexeme = assignment->getNextLexeme())) {
        next_lexeme = ctx.coalesceSymbols(next_lexeme);
        if (next_lexeme->isOperator("=")) break;

        parm.addLexeme(next_lexeme);
      }
      parm.setLexemeBOF();
      if (!exprEval.evaluate(&parm)) {
        return false;
      }

      if (next_lexeme) {
        if (next_lexeme->isOperator("=")) {
          parm.clearLexemes();
          while ((next_lexeme = assignment->getNextLexeme())) {
            next_lexeme = ctx.coalesceSymbols(next_lexeme);
            parm.addLexeme(next_lexeme);
          }
          parm.setLexemeBOF();

          result = exprEval.evaluate(&parm);

          ctx.popActionRoot();

          return result;
        }
      }
    }
  }

  ctx.error_message = "Invalid LET statement";
  return false;
}
