/***
 * @file play_statement_strategy.h
 * @brief Parser PLAY statement strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#ifndef PLAY_STATEMENT_STRATEGY_H_INCLUDED
#define PLAY_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class PlayStatementStrategy : public IParserStatementStrategy {
 public:
  bool handle(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // PLAY_STATEMENT_STRATEGY_H_INCLUDED
