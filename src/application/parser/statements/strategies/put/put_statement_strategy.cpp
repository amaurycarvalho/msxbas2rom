#include "put_statement_strategy.h"

#include "action_node.h"
#include "lexeme.h"
#include "lexer_line_context.h"
#include "logger.h"

bool PutStatementStrategy::parsePutSprite(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme;
  shared_ptr<ActionNode> action, act_coord;
  shared_ptr<LexerLineContext> parm = make_shared<LexerLineContext>();
  int state = 0, sepCount = 0;

  act_coord = make_shared<ActionNode>("COORD");

  parm->clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = context->coalesceSymbols(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator(",")) {
          if (parm->getLexemeCount()) {
            parm->setLexemeBOF();
            if (!evaluateExpression(context, parm)) {
              return false;
            }
            parm->clearLexemes();
          } else {
            context->pushActionFromLexeme(context->getLexNull());
          }
          state = 1;
          continue;
        }
      } break;

      case 1: {
        if (next_lexeme->isKeyword("STEP")) {
          action = make_shared<ActionNode>(next_lexeme);
          context->pushActionRoot(action);
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          state = 2;
          if (context->getActionRoot()) {
            if (context->getActionRoot()->lexeme) {
              if (context->getActionRoot()->lexeme->value != "STEP") {
                context->pushActionRoot(act_coord);
              }
            }
          }
          continue;
        } else if (next_lexeme->isSeparator(",")) {
          state = 3;
          context->pushActionRoot(act_coord);
          context->pushActionFromLexeme(context->getLexNull());
          context->pushActionFromLexeme(context->getLexNull());
          context->popActionRoot();
          continue;
        } else {
          context->getLogger()->error("PUT SPRITE without a valid complement");
          context->getEvalExprError() = true;
          return false;
        }
      } break;

      case 2: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount) {
            sepCount--;
          } else {
            state = 3;
            if (parm->getLexemeCount()) {
              parm->setLexemeBOF();
              if (!evaluateExpression(context, parm)) {
                return false;
              }
              parm->clearLexemes();
            } else {
              if (context->getActionRoot()) {
                context->pushActionFromLexeme(context->getLexNull());
                if (context->getActionRoot()->actions.size() == 1) {
                  context->pushActionFromLexeme(context->getLexNull());
                }
              }
            }
            context->popActionRoot();
            next_lexeme = statement->getNextLexeme();
            if (next_lexeme) {
              if (next_lexeme->type != Lexeme::type_separator ||
                  next_lexeme->value != ",") {
                return false;
              }
            }
            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm->getLexemeCount()) {
            parm->setLexemeBOF();
            if (!evaluateExpression(context, parm)) {
              return false;
            }
            parm->clearLexemes();
          } else {
            context->pushActionFromLexeme(context->getLexNull());
          }
          continue;
        }

      } break;

      case 3: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount) sepCount--;
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm->getLexemeCount()) {
            parm->setLexemeBOF();
            if (!evaluateExpression(context, parm)) {
              return false;
            }
            parm->clearLexemes();
          } else {
            context->pushActionFromLexeme(context->getLexNull());
          }
          continue;
        }

      } break;
    }

    parm->addLexeme(next_lexeme);
  }

  if (parm->getLexemeCount()) {
    parm->setLexemeBOF();
    if (!evaluateExpression(context, parm)) {
      return false;
    }

    parm->clearLexemes();
  }

  return true;
}

bool PutStatementStrategy::parsePutTile(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme;
  shared_ptr<ActionNode> action, act_coord;
  shared_ptr<LexerLineContext> parm = make_shared<LexerLineContext>();
  int state = 0, sepCount = 0;

  act_coord = make_shared<ActionNode>("COORD");

  parm->clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = context->coalesceSymbols(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator(",")) {
          if (parm->getLexemeCount()) {
            parm->setLexemeBOF();
            if (!evaluateExpression(context, parm)) {
              return false;
            }
            parm->clearLexemes();
          } else {
            context->pushActionFromLexeme(context->getLexNull());
          }
          state = 1;
          continue;
        }
      } break;

      case 1: {
        if (next_lexeme->isKeyword("STEP")) {
          action = make_shared<ActionNode>(next_lexeme);
          context->pushActionRoot(action);
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          state = 2;
          if (context->getActionRoot()) {
            if (context->getActionRoot()->lexeme) {
              if (context->getActionRoot()->lexeme->value != "STEP") {
                context->pushActionRoot(act_coord);
              }
            }
          }
          continue;
        } else if (next_lexeme->isSeparator(",")) {
          state = 3;
          context->pushActionRoot(act_coord);
          context->pushActionFromLexeme(context->getLexNull());
          context->pushActionFromLexeme(context->getLexNull());
          context->popActionRoot();
          continue;
        } else {
          context->getLogger()->error("PUT TILE without a valid complement");
          context->getEvalExprError() = true;
          return false;
        }
      } break;

      case 2: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount) {
            sepCount--;
          } else {
            state = 3;
            if (parm->getLexemeCount()) {
              parm->setLexemeBOF();
              if (!evaluateExpression(context, parm)) {
                return false;
              }
              parm->clearLexemes();
            } else {
              if (context->getActionRoot()) {
                context->pushActionFromLexeme(context->getLexNull());
                if (context->getActionRoot()->actions.size() == 1) {
                  context->pushActionFromLexeme(context->getLexNull());
                }
              }
            }
            context->popActionRoot();
            next_lexeme = statement->getNextLexeme();
            if (next_lexeme) {
              if (next_lexeme->type != Lexeme::type_separator ||
                  next_lexeme->value != ",") {
                return false;
              }
            }
            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm->getLexemeCount()) {
            parm->setLexemeBOF();
            if (!evaluateExpression(context, parm)) {
              return false;
            }
            parm->clearLexemes();
          } else {
            context->pushActionFromLexeme(context->getLexNull());
          }
          continue;
        }

      } break;

      case 3: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount) sepCount--;
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm->getLexemeCount()) {
            parm->setLexemeBOF();
            if (!evaluateExpression(context, parm)) {
              return false;
            }
            parm->clearLexemes();
          } else {
            context->pushActionFromLexeme(context->getLexNull());
          }
          continue;
        }

      } break;
    }

    parm->addLexeme(next_lexeme);
  }

  if (parm->getLexemeCount()) {
    parm->setLexemeBOF();
    if (!evaluateExpression(context, parm)) {
      return false;
    }

    parm->clearLexemes();
  }

  return true;
}

bool PutStatementStrategy::parseStatement(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme;
  shared_ptr<ActionNode> action;
  bool result = false;

  if ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = context->coalesceSymbols(next_lexeme);

    if (next_lexeme->type == Lexeme::type_keyword) {
      action = make_shared<ActionNode>(next_lexeme);
      context->pushActionRoot(action);

      if (next_lexeme->value == "SPRITE") {
        result = parsePutSprite(context, statement);
      } else if (next_lexeme->value == "TILE") {
        result = parsePutTile(context, statement);
      }

      context->popActionRoot();
    }
  }

  return result;
}

bool PutStatementStrategy::execute(shared_ptr<ParserContext> context,
                                   shared_ptr<LexerLineContext> statement,
                                   shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  return parseStatement(context, statement);
}
