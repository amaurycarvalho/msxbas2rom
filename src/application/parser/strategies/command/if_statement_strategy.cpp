/***
 * @file if_statement_strategy.cpp
 * @brief Parser IF statement strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#include "if_statement_strategy.h"

#include "parser.h"

bool IfStatementStrategy::handle(Parser& parser, LexerLine* statement,
                                 Lexeme* lexeme) {
  (void)lexeme;
  parser.statementBypassCleanup = true;
  return parser.eval_cmd_if(statement, 0);
}
