#ifndef ON_STATEMENT_STRATEGY_H_INCLUDED
#define ON_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class OnStatementStrategy : public IParserStatementStrategy {
 public:
  bool parseOn(shared_ptr<ParserContext> context,
               shared_ptr<LexerLineContext> statement);
  bool parseOnGotoGosub(shared_ptr<ParserContext> context,
                        shared_ptr<LexerLineContext> statement);
  bool parseOnError(shared_ptr<ParserContext> context,
                    shared_ptr<LexerLineContext> statement);
  bool parseOnInterval(shared_ptr<ParserContext> context,
                       shared_ptr<LexerLineContext> statement);
  bool parseOnKey(shared_ptr<ParserContext> context,
                  shared_ptr<LexerLineContext> statement);
  bool parseOnSprite(shared_ptr<ParserContext> context,
                     shared_ptr<LexerLineContext> statement);
  bool parseOnStop(shared_ptr<ParserContext> context,
                   shared_ptr<LexerLineContext> statement);
  bool parseOnStrig(shared_ptr<ParserContext> context,
                    shared_ptr<LexerLineContext> statement);

  bool parseInterval(shared_ptr<ParserContext> context,
                     shared_ptr<LexerLineContext> statement);
  bool parseStop(shared_ptr<ParserContext> context,
                 shared_ptr<LexerLineContext> statement);
  bool parseKey(shared_ptr<ParserContext> context,
                shared_ptr<LexerLineContext> statement);
  bool parseStrig(shared_ptr<ParserContext> context,
                  shared_ptr<LexerLineContext> statement);

  bool execute(shared_ptr<ParserContext> context,
               shared_ptr<LexerLineContext> statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // ON_STATEMENT_STRATEGY_H_INCLUDED
