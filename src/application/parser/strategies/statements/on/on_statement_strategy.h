#ifndef ON_STATEMENT_STRATEGY_H_INCLUDED
#define ON_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class OnStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseOn(Parser& parser, LexerLine* statement);
  bool parseOnGotoGosub(Parser& parser, LexerLine* statement);
  bool parseOnError(Parser& parser, LexerLine* statement);
  bool parseOnInterval(Parser& parser, LexerLine* statement);
  bool parseOnKey(Parser& parser, LexerLine* statement);
  bool parseOnSprite(Parser& parser, LexerLine* statement);
  bool parseOnStop(Parser& parser, LexerLine* statement);
  bool parseOnStrig(Parser& parser, LexerLine* statement);

  bool parseInterval(Parser& parser, LexerLine* statement);
  bool parseStop(Parser& parser, LexerLine* statement);
  bool parseKey(Parser& parser, LexerLine* statement);
  bool parseStrig(Parser& parser, LexerLine* statement);

  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // ON_STATEMENT_STRATEGY_H_INCLUDED
