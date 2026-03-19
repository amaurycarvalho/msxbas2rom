#ifndef DEF_STATEMENT_STRATEGY_H_INCLUDED
#define DEF_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class DefStatementStrategy : public IParserStatementStrategy {
 private:
  bool parseDefUsr(shared_ptr<ParserContext> context,
                   shared_ptr<LexerLineContext> statement);

 public:
  bool parseWithType(shared_ptr<ParserContext> context,
                     shared_ptr<LexerLineContext> statement, int vartype);
  bool execute(shared_ptr<ParserContext> context,
               shared_ptr<LexerLineContext> statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // DEF_STATEMENT_STRATEGY_H_INCLUDED
