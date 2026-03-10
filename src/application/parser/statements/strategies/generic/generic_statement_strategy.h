#ifndef GENERIC_STATEMENT_STRATEGY_H_INCLUDED
#define GENERIC_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class GenericStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(ParserContext& context, LexerLine* statement);
  bool execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // GENERIC_STATEMENT_STRATEGY_H_INCLUDED
