#ifndef PRINT_STATEMENT_STRATEGY_H_INCLUDED
#define PRINT_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class PrintStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(ParserContext& context, LexerLineContext* statement);
  bool execute(ParserContext& context, LexerLineContext* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // PRINT_STATEMENT_STRATEGY_H_INCLUDED
