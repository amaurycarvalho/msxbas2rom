#ifndef FILE_STATEMENT_STRATEGY_H_INCLUDED
#define FILE_STATEMENT_STRATEGY_H_INCLUDED

#include "parser_statement_strategy.h"

class FileStatementStrategy : public IParserStatementStrategy {
 private:
  bool parseOpen(shared_ptr<ParserContext> context,
                 shared_ptr<LexerLineContext> statement);
  bool parseClose(shared_ptr<ParserContext> context,
                  shared_ptr<LexerLineContext> statement);
  bool parseMaxfiles(shared_ptr<ParserContext> context,
                     shared_ptr<LexerLineContext> statement);

 public:
  bool execute(shared_ptr<ParserContext> context,
               shared_ptr<LexerLineContext> statement,
               shared_ptr<Lexeme> lexeme) override;
};

#endif  // FILE_STATEMENT_STRATEGY_H_INCLUDED
