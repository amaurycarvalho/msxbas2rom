#ifndef DATA_STATEMENT_STRATEGY_H_INCLUDED
#define DATA_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class DataStatementStrategy : public IParserStatementStrategy {
 protected:
  bool parseData(ParserContext& context, LexerLine* statement,
                 bool isBinaryData);

 public:
  bool execute(ParserContext& context, LexerLine* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // DATA_STATEMENT_STRATEGY_H_INCLUDED
