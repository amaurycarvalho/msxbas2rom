/***
 * @file command_statement_strategy.cpp
 * @brief Parser command statement strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#include "command_statement_strategy.h"

#include "parser.h"

CommandStatementStrategy::CommandStatementStrategy(
    bool (Parser::*handler)(LexerLine*)) {
  this->handler = handler;
}

bool CommandStatementStrategy::handle(Parser& parser, LexerLine* statement,
                                      Lexeme* lexeme) {
  (void)lexeme;
  if (!handler) return true;
  return (parser.*handler)(statement);
}
