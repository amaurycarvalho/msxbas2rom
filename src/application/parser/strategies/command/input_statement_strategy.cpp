/***
 * @file input_statement_strategy.cpp
 * @brief Parser INPUT statement strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#include "input_statement_strategy.h"

#include "parser.h"

bool InputStatementStrategy::handle(Parser& parser, LexerLine* statement,
                                    Lexeme* lexeme) {
  (void)lexeme;
  parser.has_input = true;
  return parser.eval_cmd_input(statement);
}
