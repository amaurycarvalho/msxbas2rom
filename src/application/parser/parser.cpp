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

/***
 * @name Parser class code
 */

Parser::Parser()
    : exprEval(ctx),
      assignEval(ctx, exprEval),
      lineEval(ctx, statementStrategyFactory, exprEval, assignEval),
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
      if (!lineEval.evaluateLine(lexerLine)) return false;
    }
  }

  return true;
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
