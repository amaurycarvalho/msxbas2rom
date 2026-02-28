/***
 * @file generic_statement_strategy.h
 * @brief Parser generic command strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#ifndef GENERIC_STATEMENT_STRATEGY_H_INCLUDED
#define GENERIC_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class GenericStatementStrategy : public IParserStatementStrategy {
 public:
  bool handle(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // GENERIC_STATEMENT_STRATEGY_H_INCLUDED
