/***
 * @file play_statement_strategy.cpp
 * @brief Parser PLAY statement strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#include "play_statement_strategy.h"

#include "parser.h"

bool PlayStatementStrategy::handle(Parser& parser, LexerLine* statement,
                                   Lexeme* lexeme) {
  (void)lexeme;
  parser.has_play = true;
  return parser.eval_cmd_generic(statement);
}
