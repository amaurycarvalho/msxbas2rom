#ifndef PRINT_STATEMENT_STRATEGY_H_INCLUDED
#define PRINT_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class PrintStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(ParserContext& context, LexerLine* statement);
  bool execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // PRINT_STATEMENT_STRATEGY_H_INCLUDED
