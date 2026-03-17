#ifndef PUT_STATEMENT_STRATEGY_H_INCLUDED
#define PUT_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class PutStatementStrategy : public IParserStatementStrategy {
 public:
  bool parsePutSprite(ParserContext& context, LexerLine* statement);
  bool parsePutTile(ParserContext& context, LexerLine* statement);
  bool parseStatement(ParserContext& context, LexerLine* statement);
  bool execute(ParserContext& context, LexerLine* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // PUT_STATEMENT_STRATEGY_H_INCLUDED
