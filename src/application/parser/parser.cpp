/***
 * @file parser.cpp
 * @brief Parser class code specialized as a MSX-BASIC syntax tree builder
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
#include "parser_statement_strategy_factory.h"
#include "tag_node.h"

/***
 * @name Parser class code
 */

Parser::Parser() {
  statementStrategyFactory = make_shared<ParserStatementStrategyFactory>();
  ctx = make_shared<ParserContext>();
  ctx->setHelpers(ctx, statementStrategyFactory);
}

Parser::~Parser() = default;

bool Parser::evaluate(shared_ptr<Lexer> lexer) {
  this->ctx->getLexer() = lexer;
  this->ctx->getOpts() = lexer->opts;

  ctx->getLineEval()->setSourceResolver(lexer->getSourceResolver());

  ctx->getLogger()->setFile(ctx->getOpts()->inputFilename);

  ctx->getLogger()->debug("Displaying syntactic analysis:");

  ctx->reset();
  ctx->getLineNumber() = 0;

  for (auto& lexerLine : lexer->lines) {
    ctx->getLogger()->setLineNumber(++ctx->getLineNumber());
    if (lexerLine) {
      if (lexerLine->getLexemeCount() > 0) {
        ctx->getLineComment() = false;
        if (!ctx->getLineEval()->evaluateLine(lexerLine)) {
          if (ctx->getLogger()->empty()) {
            ctx->getLogger()->error("Syntactic error");
          }
          ctx->getLogger()->info(lexerLine->toString());
          return false;
        }
        if (!ctx->getTags().empty()) {
          ctx->getLogger()->debug(ctx->getTags().back()->toString());
        }
      }
    } else
      ctx->getLogger()->warning("Cannot evaluate a null line (line number " +
                           to_string(ctx->getLineNumber()) + ")");
  }

  return true;
}

shared_ptr<Logger> Parser::getLogger() {
  return ctx->getLogger();
}

int Parser::getLineNumber() const {
  return ctx->getLineNumber();
}

vector<shared_ptr<TagNode>>& Parser::getTags() {
  return ctx->getTags();
}
const vector<shared_ptr<TagNode>>& Parser::getTags() const {
  return ctx->getTags();
}

vector<shared_ptr<Lexeme>>& Parser::getSymbolList() {
  return ctx->getSymbolList();
}
const vector<shared_ptr<Lexeme>>& Parser::getSymbolList() const {
  return ctx->getSymbolList();
}

vector<shared_ptr<Lexeme>>& Parser::getDatas() {
  return ctx->getDatas();
}
const vector<shared_ptr<Lexeme>>& Parser::getDatas() const {
  return ctx->getDatas();
}

bool Parser::getHasTraps() const {
  return ctx->getHasTraps();
}
bool Parser::getHasDefusr() const {
  return ctx->getHasDefusr();
}
bool Parser::getHasData() const {
  return ctx->getHasData();
}
bool Parser::getHasIData() const {
  return ctx->getHasIData();
}
bool Parser::getHasPlay() const {
  return ctx->getHasPlay();
}
bool Parser::getHasInput() const {
  return ctx->getHasInput();
}
bool Parser::getHasFont() const {
  return ctx->getHasFont();
}
bool Parser::getHasMtf() const {
  return ctx->getHasMtf();
}
bool Parser::getHasPt3() const {
  return ctx->getHasPt3();
}
bool Parser::getHasAkm() const {
  return ctx->getHasAkm();
}
bool Parser::getHasResourceRestore() const {
  return ctx->getHasResourceRestore();
}
bool Parser::getHasFileSupport() const {
  return ctx->getHasFileSupport();
}

int Parser::getResourceCount() const {
  return ctx->getResourceCount();
}

shared_ptr<Lexer> Parser::getLexer() const {
  return ctx->getLexer();
}
shared_ptr<BuildOptions> Parser::getOpts() const {
  return ctx->getOpts();
}

string Parser::toString() {
  string out;
  for (unsigned int i = 0; i < ctx->getTags().size(); i++) {
    if (ctx->getTags()[i]) out += ctx->getTags()[i]->toString();
  }
  return out;
}
