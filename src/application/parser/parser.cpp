/***
 * @file parser.cpp
 * @brief Parser class code specialized as a MSX BASIC syntax tree builder
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   https://en.wikipedia.org/wiki/Parsing
 *   https://en.wikipedia.org/wiki/Recursive_descent_parser
 *   https://www.strchr.com/expression_evaluator
 *   https://en.wikipedia.org/wiki/Shunting-yard_algorithm
 */

#include "parser.h"

#include "call_statement_strategy.h"
#include "cmd_statement_strategy.h"
#include "file_statement_strategy.h"
#include "get_statement_strategy.h"
#include "graphics_statement_strategy.h"
#include "on_statement_strategy.h"
#include "put_statement_strategy.h"
#include "screen_statement_strategy.h"
#include "set_statement_strategy.h"
#include "sprite_statement_strategy.h"

/***
 * @name Parser class code
 */

Parser::Parser()
    : exprEval(ctx),
      assignEval(ctx, exprEval),
      includeLoader(*this),
      tag(ctx.tag),
      actionRoot(ctx.actionRoot),
      error_line(ctx.error_line),
      lex_null(ctx.lex_null),
      lex_index(ctx.lex_index),
      lex_empty_string(ctx.lex_empty_string),
      actionStack(ctx.actionStack),
      expressionList(ctx.expressionList),
      deftbl(ctx.deftbl),
      eval_expr_error(ctx.eval_expr_error),
      line_comment(ctx.line_comment),
      error_message(ctx.error_message),
      lineNo(ctx.lineNo),
      tags(ctx.tags),
      symbolList(ctx.symbolList),
      datas(ctx.datas),
      has_traps(ctx.has_traps),
      has_defusr(ctx.has_defusr),
      has_data(ctx.has_data),
      has_idata(ctx.has_idata),
      has_play(ctx.has_play),
      has_input(ctx.has_input),
      has_font(ctx.has_font),
      has_mtf(ctx.has_mtf),
      has_pt3(ctx.has_pt3),
      has_akm(ctx.has_akm),
      has_resource_restore(ctx.has_resource_restore),
      resourceCount(ctx.resourceCount),
      lexer(0),
      opts(0) {}

Parser::~Parser() {}

ParserContext& Parser::getContext() {
  return ctx;
}

const ParserContext& Parser::getContext() const {
  return ctx;
}

Lexeme* Parser::coalesceLexeme(Lexeme* lexeme) {
  return ctx.coalesceSymbols(lexeme);
}

bool Parser::evalPhraseTokens(LexerLine* phrase) {
  return eval_phrase(phrase);
}

bool Parser::evalExpressionTokens(LexerLine* expression) {
  return exprEval.evaluate(expression);
}

bool Parser::evalAssignmentTokens(LexerLine* assignment) {
  return assignEval.evaluate(assignment);
}

bool Parser::processLine(LexerLine* line) {
  return eval_line(line);
}

int Parser::gfxOperatorFromLexeme(Lexeme* lexeme) {
  return ctx.gfxOperatorCode(lexeme);
}

void Parser::pushActionNodeRoot(ActionNode* action) {
  ctx.pushActionRoot(action);
}

ActionNode* Parser::pushActionFromLexemeNode(Lexeme* lexeme) {
  return ctx.pushActionFromLexeme(lexeme);
}

void Parser::popActionNodeRoot() {
  ctx.popActionRoot();
}

bool Parser::evaluate(Lexer* lexer) {
  int i, t = lexer->lines.size();
  LexerLine* lexerLine;

  this->lexer = lexer;
  this->opts = lexer->opts;
  ctx.reset();

  for (i = 0, ctx.lineNo = 1; i < t; i++, ctx.lineNo++) {
    lexerLine = lexer->lines[i];
    // if(opts->debug) {
    // printf("%s\n",lexerLine->line.c_str());
    // }
    if (lexerLine->getLexemeCount() > 0) {
      ctx.line_comment = false;
      if (!eval_line(lexerLine)) return false;
    }
  }

  return true;
}

bool Parser::eval_line(LexerLine* lexerLine) {
  Lexeme* lexeme = lexerLine->getFirstLexeme();
  ActionNode* action;
  LexerLine phrase;
  int if_count = 0;

  ctx.error_line = lexerLine;
  ctx.actionRoot = 0;

  if (lexeme) {
    lexeme = ctx.coalesceSymbols(lexeme);

    if (lexeme->isLiteralNumeric()) {
      ctx.tag = new TagNode();  // register line number tag
      ctx.tag->name = lexeme->value;
      ctx.tag->value = ctx.tag->name;
      ctx.tags.push_back(ctx.tag);

      while ((lexeme = lexerLine->getNextLexeme())) {
        lexeme = ctx.coalesceSymbols(lexeme);

        if (lexeme->isKeyword("IF")) {
          if_count++;
        }

        if (lexeme->isSeparator(":") && if_count == 0) {
          ctx.actionRoot = 0;
          if (phrase.getLexemeCount())
            if (!eval_phrase(&phrase)) return false;
          phrase.clearLexemes();
          continue;

        } else if (lexeme->isOperator("'")) {
          break;

        } else {
          phrase.addLexeme(lexeme);
        }
      }

      if (phrase.getLexemeCount()) {
        ctx.actionRoot = 0;
        if (!eval_phrase(&phrase)) return false;
      }

    } else if (lexeme->type == Lexeme::type_keyword) {
      if (lexeme->value == "FILE" || lexeme->value == "TEXT") {
        ctx.resourceCount++;

        ctx.tag = new TagNode();  // register line number tag
        ctx.tag->name = "DIRECTIVE";
        ctx.tag->value = ctx.tag->name;
        ctx.tags.push_back(ctx.tag);

        action = new ActionNode(lexeme);
        ctx.pushActionRoot(action);

        lexeme = lexerLine->getNextLexeme();
        if (lexeme) ctx.pushActionFromLexeme(lexeme);

        ctx.popActionRoot();

      } else if (lexeme->value == "INCLUDE") {
        if ((lexeme = lexerLine->getNextLexeme())) {
          if (lexeme->type == Lexeme::type_literal &&
              lexeme->subtype == Lexeme::subtype_string) {
            if (!includeLoader.load(lexeme)) {
              ctx.error_message =
                  "INCLUDE file not found or with content syntax error";
              return false;
            }

          } else {
            ctx.error_message = "Invalid parameter in INCLUDE keyword";
            return false;
          }
        }
      }
    }
  }

  return true;
}

bool Parser::eval_phrase(LexerLine* phrase) {
  Lexeme* lexeme = phrase->getFirstLexeme();

  if (lexeme) {
    lexeme = ctx.coalesceSymbols(lexeme);

    if (lexeme->type == Lexeme::type_identifier ||
        (lexeme->type == Lexeme::type_keyword &&
         lexeme->subtype == Lexeme::subtype_function &&
         lexeme->value != "STRIG")) {
      phrase->setLexemeBOF();
      return assignEval.evaluate(phrase);
    } else if (lexeme->type == Lexeme::type_keyword) {
      return eval_statement(phrase);
    } else if (lexeme->isOperator("'")) {
      return eval_statement(phrase);
    } else if (lexeme->isOperator("_")) {
      return eval_statement(phrase);
    }
  }

  ctx.error_message = "Invalid keyword/identifier";
  return false;
}

bool Parser::eval_statement(LexerLine* statement) {
  Lexeme* lexeme;
  ActionNode* action;
  IParserStatementStrategy* strategy;
  ActionNode* actionSaved = ctx.actionRoot;
  unsigned int actionCount = ctx.actionStack.size();
  bool result = true;

  lexeme = statement->getFirstLexeme();

  if (lexeme) {
    lexeme = ctx.coalesceSymbols(lexeme);

    action = new ActionNode(lexeme);
    ctx.pushActionRoot(action);

    strategy = statementStrategyFactory.getStrategyByKeyword(lexeme->value);
    if (strategy) {
      result = strategy->execute(*this, statement, lexeme);
      if (lexeme->value == "IF") return result;
    } else {
      ctx.error_message = "Invalid keyword / identifier";
      result = false;
    }

    ctx.popActionRoot();
  }

  while (ctx.actionStack.size() > actionCount) ctx.actionStack.pop();

  ctx.actionRoot = actionSaved;

  return result;
}

string Parser::toString() {
  string out;
  for (unsigned int i = 0; i < ctx.tags.size(); i++) {
    if (ctx.tags[i]) out += ctx.tags[i]->toString();
  }
  return out;
}

string Parser::errorToString() {
  string out;
  if (ctx.error_line) out += ctx.error_line->toString();
  if (ctx.error_message.size() > 0) {
    out += ctx.error_message;
    out += "\n";
  }
  return out;
}
