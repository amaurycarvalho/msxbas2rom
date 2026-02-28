/***
 * @file command_statement_strategy.cpp
 * @brief Parser command statement strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#include "command_statement_strategy.h"

#include "parser.h"

CommandStatementStrategy::CommandStatementStrategy(
    ParserStatementAction action) {
  this->action = action;
}

bool CommandStatementStrategy::handle(Parser& parser, LexerLine* statement,
                                      Lexeme* lexeme) {
  return parser.executeStatementCommand(action, statement, lexeme);
}
