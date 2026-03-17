#ifndef ON_STATEMENT_STRATEGY_H_INCLUDED
#define ON_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class OnStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseOn(ParserContext& context, LexerLineContext* statement);
  bool parseOnGotoGosub(ParserContext& context, LexerLineContext* statement);
  bool parseOnError(ParserContext& context, LexerLineContext* statement);
  bool parseOnInterval(ParserContext& context, LexerLineContext* statement);
  bool parseOnKey(ParserContext& context, LexerLineContext* statement);
  bool parseOnSprite(ParserContext& context, LexerLineContext* statement);
  bool parseOnStop(ParserContext& context, LexerLineContext* statement);
  bool parseOnStrig(ParserContext& context, LexerLineContext* statement);

  bool parseInterval(ParserContext& context, LexerLineContext* statement);
  bool parseStop(ParserContext& context, LexerLineContext* statement);
  bool parseKey(ParserContext& context, LexerLineContext* statement);
  bool parseStrig(ParserContext& context, LexerLineContext* statement);

  bool execute(ParserContext& context, LexerLineContext* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // ON_STATEMENT_STRATEGY_H_INCLUDED
