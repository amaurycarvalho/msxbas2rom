#ifndef ON_STATEMENT_STRATEGY_H_INCLUDED
#define ON_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class OnStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseOn(ParserContext& context, LexerLine* statement);
  bool parseOnGotoGosub(ParserContext& context, LexerLine* statement);
  bool parseOnError(ParserContext& context, LexerLine* statement);
  bool parseOnInterval(ParserContext& context, LexerLine* statement);
  bool parseOnKey(ParserContext& context, LexerLine* statement);
  bool parseOnSprite(ParserContext& context, LexerLine* statement);
  bool parseOnStop(ParserContext& context, LexerLine* statement);
  bool parseOnStrig(ParserContext& context, LexerLine* statement);

  bool parseInterval(ParserContext& context, LexerLine* statement);
  bool parseStop(ParserContext& context, LexerLine* statement);
  bool parseKey(ParserContext& context, LexerLine* statement);
  bool parseStrig(ParserContext& context, LexerLine* statement);

  bool execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // ON_STATEMENT_STRATEGY_H_INCLUDED
