/***
 * @file input_statement_strategy.h
 * @brief Parser INPUT statement strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#ifndef INPUT_STATEMENT_STRATEGY_H_INCLUDED
#define INPUT_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class InputStatementStrategy : public IParserStatementStrategy {
 public:
  bool handle(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // INPUT_STATEMENT_STRATEGY_H_INCLUDED
