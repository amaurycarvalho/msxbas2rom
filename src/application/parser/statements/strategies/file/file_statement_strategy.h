#ifndef FILE_STATEMENT_STRATEGY_H_INCLUDED
#define FILE_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class FileStatementStrategy : public IParserStatementStrategy {
 private:
  bool parseOpen(ParserContext& context, LexerLineContext* statement);
  bool parseClose(ParserContext& context, LexerLineContext* statement);
  bool parseMaxfiles(ParserContext& context, LexerLineContext* statement);

 public:
  bool execute(ParserContext& context, LexerLineContext* statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // FILE_STATEMENT_STRATEGY_H_INCLUDED
