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

#include "logger.h"

/***
 * @name Parser class code
 */

Parser::Parser()
    : exprEval(ctx),
      assignEval(ctx, exprEval),
      lineEval(ctx, statementStrategyFactory, exprEval, assignEval),
      lexer(nullptr),
      opts(nullptr) {}

Parser::~Parser() = default;

bool Parser::evaluate(Lexer* lexer) {
  int i, t = lexer->lines.size();
  LexerLine* lexerLine;

  this->lexer = lexer;
  this->opts = lexer->opts;

  ctx.logger->setFile(opts->inputFilename);

  ctx.logger->debug("Displaying syntactic analysis:");

  ctx.reset();

  for (i = 0, ctx.lineNumber = 1; i < t; i++, ctx.lineNumber++) {
    ctx.logger->setLineNumber(ctx.lineNumber);
    lexerLine = lexer->lines[i];
    if (lexerLine->getLexemeCount() > 0) {
      ctx.line_comment = false;
      if (!lineEval.evaluateLine(lexerLine)) {
        if (ctx.logger->empty()) {
          ctx.logger->error("Syntactic error");
        }
        ctx.logger->info(lexerLine->toString());
        return false;
      }
      if (!ctx.tags.empty()) {
        ctx.logger->debug(lexerLine->line + ctx.tags.back()->toString());
      }
    }
  }

  return true;
}

Logger* Parser::getLogger() {
  return ctx.logger.get();
}

int Parser::getLineNumber() const {
  return ctx.lineNumber;
}

vector<TagNode*>& Parser::getTags() {
  return ctx.tags;
}
const vector<TagNode*>& Parser::getTags() const {
  return ctx.tags;
}

vector<Lexeme*>& Parser::getSymbolList() {
  return ctx.symbolList;
}
const vector<Lexeme*>& Parser::getSymbolList() const {
  return ctx.symbolList;
}

vector<Lexeme*>& Parser::getDatas() {
  return ctx.datas;
}
const vector<Lexeme*>& Parser::getDatas() const {
  return ctx.datas;
}

bool Parser::getHasTraps() const {
  return ctx.has_traps;
}
bool Parser::getHasDefusr() const {
  return ctx.has_defusr;
}
bool Parser::getHasData() const {
  return ctx.has_data;
}
bool Parser::getHasIData() const {
  return ctx.has_idata;
}
bool Parser::getHasPlay() const {
  return ctx.has_play;
}
bool Parser::getHasInput() const {
  return ctx.has_input;
}
bool Parser::getHasFont() const {
  return ctx.has_font;
}
bool Parser::getHasMtf() const {
  return ctx.has_mtf;
}
bool Parser::getHasPt3() const {
  return ctx.has_pt3;
}
bool Parser::getHasAkm() const {
  return ctx.has_akm;
}
bool Parser::getHasResourceRestore() const {
  return ctx.has_resource_restore;
}

int Parser::getResourceCount() const {
  return ctx.resourceCount;
}

Lexer* Parser::getLexer() const {
  return lexer;
}
BuildOptions* Parser::getOpts() const {
  return opts;
}

string Parser::toString() {
  string out;
  for (unsigned int i = 0; i < ctx.tags.size(); i++) {
    if (ctx.tags[i]) out += ctx.tags[i]->toString();
  }
  return out;
}
