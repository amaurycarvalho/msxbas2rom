#ifndef DEF_STATEMENT_STRATEGY_H_INCLUDED
#define DEF_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class DefStatementStrategy : public IParserStatementStrategy {
 private:
  bool parseDefUsr(ParserContext& context, LexerLineContext* statement);

 public:
  bool parseWithType(ParserContext& context, LexerLineContext* statement,
                     int vartype);
  bool execute(ParserContext& context, LexerLineContext* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // DEF_STATEMENT_STRATEGY_H_INCLUDED
