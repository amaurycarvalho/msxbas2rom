/***
 * @file alias_statement_strategy.h
 * @brief Parser alias statement strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#ifndef ALIAS_STATEMENT_STRATEGY_H_INCLUDED
#define ALIAS_STATEMENT_STRATEGY_H_INCLUDED

#include <string>

#include "parser_statement_strategy.h"

using namespace std;

class AliasStatementStrategy : public IParserStatementStrategy {
 private:
  string aliasValue;
  ParserStatementAction action;
  bool promoteToKeyword;

 public:
  AliasStatementStrategy(string aliasValue, ParserStatementAction action,
                         bool promoteToKeyword);
  bool handle(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // ALIAS_STATEMENT_STRATEGY_H_INCLUDED
