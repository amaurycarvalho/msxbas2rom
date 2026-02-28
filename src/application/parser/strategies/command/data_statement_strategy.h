/***
 * @file data_statement_strategy.h
 * @brief Parser DATA statement strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#ifndef DATA_STATEMENT_STRATEGY_H_INCLUDED
#define DATA_STATEMENT_STRATEGY_H_INCLUDED

#include "lexeme.h"
#include "parser_statement_strategy.h"

class DataStatementStrategy : public IParserStatementStrategy {
 private:
  Lexeme::LexemeSubType subtype;

 public:
  explicit DataStatementStrategy(Lexeme::LexemeSubType subtype);
  bool handle(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // DATA_STATEMENT_STRATEGY_H_INCLUDED
