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

#include "action_node.h"
#include "assignment_evaluator.h"
#include "build_options.h"
#include "expression_evaluator.h"
#include "lexer.h"
#include "lexer_line_evaluator.h"
#include "logger.h"
#include "parser_context.h"
#include "parser_line_evaluator.h"
#include "tag_node.h"

/***
 * @name Parser class code
 */

Parser::Parser() {
  ctx = make_shared<ParserContext>();
  ctx->setHelpers(ctx, &statementStrategyFactory);
}

Parser::~Parser() = default;

bool Parser::evaluate(shared_ptr<Lexer> lexer) {
  this->ctx->lexer = lexer;
  this->ctx->opts = lexer->opts;

  ctx->logger->setFile(ctx->opts->inputFilename);

  ctx->logger->debug("Displaying syntactic analysis:");

  ctx->reset();
  ctx->lineNumber = 0;

  for (auto& lexerLine : lexer->lines) {
    ctx->logger->setLineNumber(++ctx->lineNumber);
    if (lexerLine) {
      if (lexerLine->getLexemeCount() > 0) {
        ctx->line_comment = false;
        if (!ctx->lineEval->evaluateLine(lexerLine)) {
          if (ctx->logger->empty()) {
            ctx->logger->error("Syntactic error");
          }
          ctx->logger->info(lexerLine->toString());
          return false;
        }
        if (!ctx->tags.empty()) {
          ctx->logger->debug(ctx->tags.back()->toString());
        }
      }
    } else
      ctx->logger->warning("Cannot evaluate a null line (line number " +
                           to_string(ctx->lineNumber) + ")");
  }

  return true;
}

shared_ptr<Logger> Parser::getLogger() {
  return ctx->logger;
}

int Parser::getLineNumber() const {
  return ctx->lineNumber;
}

vector<shared_ptr<TagNode>>& Parser::getTags() {
  return ctx->tags;
}
const vector<shared_ptr<TagNode>>& Parser::getTags() const {
  return ctx->tags;
}

vector<shared_ptr<Lexeme>>& Parser::getSymbolList() {
  return ctx->symbolList;
}
const vector<shared_ptr<Lexeme>>& Parser::getSymbolList() const {
  return ctx->symbolList;
}

vector<shared_ptr<Lexeme>>& Parser::getDatas() {
  return ctx->datas;
}
const vector<shared_ptr<Lexeme>>& Parser::getDatas() const {
  return ctx->datas;
}

bool Parser::getHasTraps() const {
  return ctx->has_traps;
}
bool Parser::getHasDefusr() const {
  return ctx->has_defusr;
}
bool Parser::getHasData() const {
  return ctx->has_data;
}
bool Parser::getHasIData() const {
  return ctx->has_idata;
}
bool Parser::getHasPlay() const {
  return ctx->has_play;
}
bool Parser::getHasInput() const {
  return ctx->has_input;
}
bool Parser::getHasFont() const {
  return ctx->has_font;
}
bool Parser::getHasMtf() const {
  return ctx->has_mtf;
}
bool Parser::getHasPt3() const {
  return ctx->has_pt3;
}
bool Parser::getHasAkm() const {
  return ctx->has_akm;
}
bool Parser::getHasResourceRestore() const {
  return ctx->has_resource_restore;
}

int Parser::getResourceCount() const {
  return ctx->resourceCount;
}

shared_ptr<Lexer> Parser::getLexer() const {
  return ctx->lexer;
}
shared_ptr<BuildOptions> Parser::getOpts() const {
  return ctx->opts;
}

string Parser::toString() {
  string out;
  for (unsigned int i = 0; i < ctx->tags.size(); i++) {
    if (ctx->tags[i]) out += ctx->tags[i]->toString();
  }
  return out;
}
