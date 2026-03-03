#include "idata_statement_strategy.h"

#include "lexer.h"
#include "parser.h"

bool IDataStatementStrategy::execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) {
  (void)lexeme;
  return parseData(context, statement, true);
}
