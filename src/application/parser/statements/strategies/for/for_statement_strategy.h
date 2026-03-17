#ifndef FOR_STATEMENT_STRATEGY_H_INCLUDED
#define FOR_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class ForStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(ParserContext& context, LexerLineContext* statement);
  bool execute(ParserContext& context, LexerLineContext* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // FOR_STATEMENT_STRATEGY_H_INCLUDED
