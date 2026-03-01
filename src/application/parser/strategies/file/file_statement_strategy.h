#ifndef FILE_STATEMENT_STRATEGY_H_INCLUDED
#define FILE_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class FileStatementStrategy : public IParserStatementStrategy {
 private:
  bool parseOpen(Parser& parser, LexerLine* statement);
  bool parseClose(Parser& parser, LexerLine* statement);
  bool parseMaxfiles(Parser& parser, LexerLine* statement);

 public:
  bool execute(Parser& parser, LexerLine* statement, Lexeme* lexeme) override;
};

#endif  // FILE_STATEMENT_STRATEGY_H_INCLUDED
