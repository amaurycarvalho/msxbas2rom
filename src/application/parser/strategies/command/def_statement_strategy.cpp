/***
 * @file def_statement_strategy.cpp
 * @brief Parser DEF statement strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#include "def_statement_strategy.h"

#include "parser.h"

DefStatementStrategy::DefStatementStrategy(int varType) {
  this->varType = varType;
}

bool DefStatementStrategy::handle(Parser& parser, LexerLine* statement,
                                  Lexeme* lexeme) {
  (void)lexeme;
  return parser.eval_cmd_def(statement, varType);
}
