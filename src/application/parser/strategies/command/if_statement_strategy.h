/***
 * @file if_statement_strategy.h
 * @brief Parser IF statement strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#ifndef IF_STATEMENT_STRATEGY_H_INCLUDED
#define IF_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class IfStatementStrategy : public IParserStatementStrategy {
 public:
  bool handle(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // IF_STATEMENT_STRATEGY_H_INCLUDED
