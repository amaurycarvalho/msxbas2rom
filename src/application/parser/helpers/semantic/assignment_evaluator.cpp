/***
 * @file assignment_evaluator.cpp
 * @brief Assignments syntatic analysis class code
 * @author Amaury Carvalho (2019-2026)
 */

#include "assignment_evaluator.h"

#include "action_node.h"
#include "expression_evaluator.h"
#include "lexeme.h"
#include "lexer_line_evaluator.h"
#include "logger.h"
#include "parser_context.h"

AssignmentEvaluator::AssignmentEvaluator(
    shared_ptr<ParserContext> context, shared_ptr<ExpressionEvaluator> exprEval)
    : ctx(context), exprEval(exprEval) {}

AssignmentEvaluator::~AssignmentEvaluator() = default;

bool AssignmentEvaluator::evaluate(shared_ptr<LexerLineContext> assignment) {
  shared_ptr<Lexeme> lexeme = assignment->getNextLexeme(), next_lexeme;
  shared_ptr<LexerLineContext> parm = make_shared<LexerLineContext>();
  shared_ptr<ActionNode> action;
  shared_ptr<Lexeme> lexLet;
  bool result, add_let_action = true;

  if (lexeme) {
    lexeme = ctx->coalesceSymbols(lexeme);

    if (lexeme->type == Lexeme::type_identifier ||
        (lexeme->type == Lexeme::type_keyword &&
         lexeme->subtype == Lexeme::subtype_function)) {
      if (ctx->actionRoot) {
        lexLet = ctx->actionRoot->lexeme;
        if (lexLet->isKeyword("LET")) {
          add_let_action = false;
        }
      }

      if (add_let_action) {
        action = make_shared<ActionNode>("LET");
        /// @note "lexLet" value really needs to be updated?
        /// NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
        // lexLet = action->lexeme;
        ctx->pushActionRoot(action);
      }

      parm->clearLexemes();
      parm->addLexeme(lexeme);
      while ((next_lexeme = assignment->getNextLexeme())) {
        next_lexeme = ctx->coalesceSymbols(next_lexeme);
        if (next_lexeme->isOperator("=")) break;

        parm->addLexeme(next_lexeme);
      }
      parm->setLexemeBOF();
      if (!exprEval->evaluate(parm)) {
        return false;
      }

      if (next_lexeme) {
        if (next_lexeme->isOperator("=")) {
          parm->clearLexemes();
          while ((next_lexeme = assignment->getNextLexeme())) {
            next_lexeme = ctx->coalesceSymbols(next_lexeme);
            parm->addLexeme(next_lexeme);
          }
          parm->setLexemeBOF();

          result = exprEval->evaluate(parm);

          ctx->popActionRoot();

          return result;
        }
      }
    }
  }

  ctx->logger->error("Invalid LET statement");

  return false;
}

shared_ptr<ParserContext> AssignmentEvaluator::getContext() {
  return ctx;
}

void AssignmentEvaluator::setContext(shared_ptr<ParserContext> context) {
  this->ctx = context;
}
