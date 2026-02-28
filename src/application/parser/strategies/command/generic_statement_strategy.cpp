/***
 * @file generic_statement_strategy.cpp
 * @brief Parser generic command strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#include "generic_statement_strategy.h"

#include "parser.h"

bool GenericStatementStrategy::handle(Parser& parser, LexerLine* statement,
                                      Lexeme* lexeme) {
  if (lexeme && lexeme->value == "BLOAD") parser.resourceCount++;
  return parser.eval_cmd_generic(statement);
}
