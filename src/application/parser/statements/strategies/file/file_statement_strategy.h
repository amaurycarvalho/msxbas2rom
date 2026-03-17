#ifndef FILE_STATEMENT_STRATEGY_H_INCLUDED
#define FILE_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class FileStatementStrategy : public IParserStatementStrategy {
 private:
  bool parseOpen(ParserContext& context, LexerLine* statement);
  bool parseClose(ParserContext& context, LexerLine* statement);
  bool parseMaxfiles(ParserContext& context, LexerLine* statement);

 public:
  bool execute(ParserContext& context, LexerLine* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // FILE_STATEMENT_STRATEGY_H_INCLUDED
