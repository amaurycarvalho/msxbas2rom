#ifndef IF_STATEMENT_STRATEGY_H_INCLUDED
#define IF_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class IfStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseStatement(ParserContext& context, LexerLineContext* statement,
                      int level);
  bool execute(ParserContext& context, LexerLineContext* statement,
               shared_ptr<Lexeme> lexeme) override;

 private:
  bool evalPhrase(ParserContext& context, LexerLineContext* phrase);
};

#endif  // IF_STATEMENT_STRATEGY_H_INCLUDED
