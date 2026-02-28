/***
 * @file command_statement_strategy.h
 * @brief Parser command statement strategy
 * @author Amaury Carvalho (2019-2026)
 * @note
 */

#ifndef COMMAND_STATEMENT_STRATEGY_H_INCLUDED
#define COMMAND_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class CommandStatementStrategy : public IParserStatementStrategy {
 private:
  ParserStatementAction action;

 public:
  explicit CommandStatementStrategy(ParserStatementAction action);
  bool handle(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // COMMAND_STATEMENT_STRATEGY_H_INCLUDED
