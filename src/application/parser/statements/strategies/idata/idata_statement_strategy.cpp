#include "idata_statement_strategy.h"

#include "lexer.h"

bool IDataStatementStrategy::execute(shared_ptr<ParserContext> context,
                                     shared_ptr<LexerLineContext> statement,
                                     shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  return parseData(context, statement, true);
}
