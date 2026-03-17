#include "idata_statement_strategy.h"

#include "lexer.h"

bool IDataStatementStrategy::execute(ParserContext& context,
                                     LexerLineContext* statement,
                                     shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  return parseData(context, statement, true);
}
