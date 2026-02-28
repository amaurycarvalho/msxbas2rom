/***
 * @file def_statement_strategy.h
 * @brief Parser DEF statement strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#ifndef DEF_STATEMENT_STRATEGY_H_INCLUDED
#define DEF_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class DefStatementStrategy : public IParserStatementStrategy {
 private:
  int varType;

 public:
  explicit DefStatementStrategy(int varType);
  bool handle(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // DEF_STATEMENT_STRATEGY_H_INCLUDED
