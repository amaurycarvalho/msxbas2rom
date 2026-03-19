#ifndef DATA_STATEMENT_STRATEGY_H_INCLUDED
#define DATA_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class DataStatementStrategy : public IParserStatementStrategy {
 protected:
  bool parseData(shared_ptr<ParserContext> context,
                 shared_ptr<LexerLineContext> statement, bool isBinaryData);

 public:
  bool execute(shared_ptr<ParserContext> context,
               shared_ptr<LexerLineContext> statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // DATA_STATEMENT_STRATEGY_H_INCLUDED
