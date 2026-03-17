#ifndef IF_STATEMENT_STRATEGY_H_INCLUDED
#define IF_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class IfStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(ParserContext& context, LexerLine* statement, int level);
  bool execute(ParserContext& context, LexerLine* statement,
               shared_ptr<Lexeme> lexeme) override;

 private:
  bool evalPhrase(ParserContext& context, LexerLine* phrase);
};

#endif  // IF_STATEMENT_STRATEGY_H_INCLUDED
