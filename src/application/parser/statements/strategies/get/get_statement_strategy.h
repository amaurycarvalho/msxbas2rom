#ifndef GET_STATEMENT_STRATEGY_H_INCLUDED
#define GET_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class GetStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseGetTile(ParserContext& context, LexerLineContext* statement);
  bool parseGetSprite(ParserContext& context, LexerLineContext* statement);
  bool parseStatement(ParserContext& context, LexerLineContext* statement);
  bool execute(ParserContext& context, LexerLineContext* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // GET_STATEMENT_STRATEGY_H_INCLUDED
